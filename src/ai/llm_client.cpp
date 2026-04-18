#include "llm_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>
#include <QRandomGenerator>
#include <QLocale>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>
#include <QUuid>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ai {

LLMClient::LLMClient(Backend backend, QObject* parent)
    : QObject(parent), backend_(backend) {
    // Generate unique session ID for this program instance
    session_id_ = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

LLMClient::~LLMClient() {
}

void LLMClient::setApiKey(const std::string& key) {
    api_key_ = key;
}

void LLMClient::setApiSecret(const std::string& secret) {
    api_secret_ = secret;
}

void LLMClient::setAppId(const std::string& app_id) {
    app_id_ = app_id;
}

void LLMClient::stop() {
    should_stop_ = true;
}
void LLMClient::setLocalModelPath(const std::string& path) {
    local_model_path_ = path;
}

void LLMClient::setBackend(Backend backend) {
    backend_ = backend;
}

bool LLMClient::isReady() const {
    return !api_key_.empty() && !api_secret_.empty() && !app_id_.empty();
}

std::string LLMClient::complete(const std::string& prompt, const std::string& model) {
    std::string result;
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    
    completeStream(prompt, [&](const StreamChunk& chunk) {
        std::lock_guard<std::mutex> lock(mtx);
        result += chunk.text;
        if (chunk.is_finish) {
            done = true;
            cv.notify_one();
        }
    }, model);
    
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [&] { return done; });
    
    return result;
}

std::future<std::string> LLMClient::completeAsync(const std::string& prompt) {
    return std::async(std::launch::async, [this, prompt]() {
        return this->complete(prompt);
    });
}

void LLMClient::completeStream(const std::string& prompt, StreamCallback on_chunk, const std::string& model) {
    switch (backend_) {
        case Backend::OpenAI:
            requestOpenAI(prompt, on_chunk);
            break;
        case Backend::Xunfei:
            (void)QtConcurrent::run([this, prompt, on_chunk]() {
                this->requestXunfei(prompt, on_chunk);
            });
            break;
        case Backend::Mock:
            requestMock(prompt, on_chunk);
            break;
        default:
            on_chunk(StreamChunk{"Unsupported backend", true, 0.0f});
            break;
    }
}

void LLMClient::requestMock(const std::string& prompt, StreamCallback cb) {
    (void)QtConcurrent::run([cb, prompt]() {
        std::string mock_response = std::string(reinterpret_cast<const char*>(u8"Mock AI response. Your prompt: ")) + prompt;
        
        // Stream character by character
        for (size_t i = 0; i < mock_response.size(); ) {
            size_t char_len = 1;
            unsigned char c = static_cast<unsigned char>(mock_response[i]);
            if ((c & 0x80) != 0) {
                if ((c & 0xE0) == 0xC0) char_len = 2;
                else if ((c & 0xF0) == 0xE0) char_len = 3;
                else if ((c & 0xF8) == 0xF0) char_len = 4;
            }
            
            std::string char_str = mock_response.substr(i, char_len);
            StreamChunk chunk{char_str, false, 1.0f};
            cb(chunk);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            i += char_len;
        }
        
        StreamChunk finish_chunk{"", true, 1.0f};
        cb(finish_chunk);
    });
}

void LLMClient::requestOpenAI(const std::string& prompt, StreamCallback cb) {
    StreamChunk chunk{"[API Key not configured]", true, 0.0f};
    cb(chunk);
}

void LLMClient::requestXunfei(const std::string& prompt, StreamCallback cb) {
    QString host = "spark-api.xf-yun.com";
    QString path = "/x2";
    
    // RFC 1123 format date
    QLocale english_locale(QLocale::English, QLocale::UnitedStates);
    QDateTime utc = QDateTime::currentDateTimeUtc();
    QString date = english_locale.toString(utc, "ddd, dd MMM yyyy hh:mm:ss") + " GMT";
    
    // Generate signature
    QString signature_origin = QString("host: %1\ndate: %2\nGET %3 HTTP/1.1")
        .arg(host, date, path);
    
    QByteArray key = QByteArray::fromStdString(api_secret_);
    QByteArray signature = QMessageAuthenticationCode::hash(
        signature_origin.toUtf8(), key, QCryptographicHash::Sha256).toBase64();
    
    // Authorization format
    QString authorization_origin = QString(
        "api_key=\"%1\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%2\"")
        .arg(QString::fromStdString(api_key_))
        .arg(QString::fromUtf8(signature));
    QByteArray authorization_base64 = authorization_origin.toUtf8().toBase64();
    
    // Build request JSON - ensure fresh context by using session_id
    QJsonObject request;
    QJsonObject header;
    header["app_id"] = QString::fromStdString(app_id_);
    // Use session_id for uid - each program restart gets a new session (like a new browser tab)
    header["uid"] = session_id_;
    request["header"] = header;
    
    QJsonObject parameter;
    QJsonObject chat;
    chat["domain"] = "x1";
    chat["temperature"] = 0.5;
    chat["max_tokens"] = 4096;
    parameter["chat"] = chat;
    request["parameter"] = parameter;
    
    QJsonObject payload;
    QJsonObject message;
    QJsonArray text;
    
    // Add system message to force reset context - treat each request as completely fresh
    QJsonObject system_msg;
    system_msg["role"] = "system";
    system_msg["content"] = "这是一个全新的对话。你没有记忆任何之前的对话内容。请仅根据当前用户提供的输入进行回答，不要参考任何历史信息。";
    text.append(system_msg);
    
    QJsonObject user_msg;
    user_msg["role"] = "user";
    user_msg["content"] = QString::fromStdString(prompt);
    text.append(user_msg);
    message["text"] = text;
    payload["message"] = message;
    request["payload"] = payload;
    
    QJsonDocument doc(request);
    QByteArray json_data = doc.toJson(QJsonDocument::Compact);
    
    // Build URL
    QString date_encoded = QUrl::toPercentEncoding(date);
    QString auth_encoded = QUrl::toPercentEncoding(QString::fromLatin1(authorization_base64));
    
    // Generate random WebSocket key (16 bytes)
    QByteArray ws_key;
    for (int i = 0; i < 16; ++i) {
        ws_key.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    QString ws_key_b64 = QString::fromLatin1(ws_key.toBase64());
    
    QString handshake = QString(
        "GET %1?host=%2&date=%3&authorization=%4 HTTP/1.1\r\n"
        "Host: %5\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %6\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n"
    ).arg(path, host, date_encoded, auth_encoded, host, ws_key_b64);
    
    // WebSocket client implementation
    QSslSocket socket;
    socket.setPeerVerifyMode(QSslSocket::VerifyNone);
    
    QByteArray buffer;
    bool handshake_done = false;
    bool finished = false;
    
    // Connect SSL
    socket.connectToHostEncrypted(host, 443);
    
    // Wait for encrypted connection
    if (!socket.waitForEncrypted(10000)) {
        StreamChunk chunk{"SSL connection timeout", true, 0.0f};
        cb(chunk);
        return;
    }
    
    // Send handshake
    socket.write(handshake.toUtf8());
    socket.flush();
    
    // Wait for response with longer timeout for large responses
    int timeout_count = 0;
    const int MAX_TIMEOUT = 600; // 60 seconds for long responses
    
    // Reset stop flag at start of request
    should_stop_ = false;
    
    while (!finished && timeout_count < MAX_TIMEOUT) {
        // Check if stop was requested
        if (should_stop_) {
            socket.close();
            StreamChunk chunk{"", true, 1.0f};
            cb(chunk);
            return;
        }
        
        socket.waitForReadyRead(100);
        
        while (socket.bytesAvailable() > 0 && !finished) {
            buffer += socket.readAll();
            
            if (!handshake_done) {
                int header_end = buffer.indexOf("\r\n\r\n");
                if (header_end > 0) {
                    QString http_response = QString::fromUtf8(buffer.left(header_end));
                    if (http_response.contains("101") && http_response.contains("Switching Protocols")) {
                        handshake_done = true;
                        buffer.remove(0, header_end + 4);
                        
                        // Send WebSocket text frame
                        QByteArray mask;
                        for (int i = 0; i < 4; ++i) {
                            mask.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
                        }
                        
                        quint64 len = json_data.size();
                        QByteArray frame;
                        frame.append(static_cast<char>(0x81)); // FIN=1, opcode=text
                        
                        if (len <= 125) {
                            frame.append(static_cast<char>(0x80 | static_cast<quint8>(len)));
                        } else if (len <= 65535) {
                            frame.append(static_cast<char>(0x80 | 126));
                            frame.append(static_cast<char>((len >> 8) & 0xFF));
                            frame.append(static_cast<char>(len & 0xFF));
                        } else {
                            frame.append(static_cast<char>(0x80 | 127));
                            for (int i = 7; i >= 0; --i) {
                                frame.append(static_cast<char>((len >> (i * 8)) & 0xFF));
                            }
                        }
                        
                        frame.append(mask);
                        for (quint64 i = 0; i < len; ++i) {
                            frame.append(json_data[i] ^ mask[i % 4]);
                        }
                        
                        socket.write(frame);
                        socket.flush();
                    } else {
                        StreamChunk chunk{"WebSocket handshake failed: " + http_response.toStdString(), true, 0.0f};
                        cb(chunk);
                        finished = true;
                        socket.close();
                    }
                }
            } else {
                // Parse WebSocket frames
                while (buffer.size() >= 2 && !finished) {
                    quint8 first = static_cast<quint8>(buffer[0]);
                    quint8 second = static_cast<quint8>(buffer[1]);
                    quint8 opcode = first & 0x0F;
                    bool masked = (second & 0x80) != 0;
                    quint64 payload_len = second & 0x7F;
                    
                    int header_len = 2;
                    if (payload_len == 126) {
                        if (buffer.size() < 4) break;
                        payload_len = (static_cast<quint8>(buffer[2]) << 8) | static_cast<quint8>(buffer[3]);
                        header_len = 4;
                    } else if (payload_len == 127) {
                        if (buffer.size() < 10) break;
                        payload_len = 0;
                        for (int i = 0; i < 8; ++i) {
                            payload_len = (payload_len << 8) | static_cast<quint8>(buffer[2 + i]);
                        }
                        header_len = 10;
                    }
                    
                    if (masked) header_len += 4;
                    if (buffer.size() < static_cast<int>(header_len + payload_len)) break;
                    
                    QByteArray mask_key = masked ? buffer.mid(header_len - 4, 4) : QByteArray();
                    QByteArray payload = buffer.mid(header_len, payload_len);
                    buffer.remove(0, header_len + payload_len);
                    
                    if (masked) {
                        for (quint64 i = 0; i < payload_len; ++i) {
                            payload[i] ^= mask_key[i % 4];
                        }
                    }
                    
                    if (opcode == 0x01 || opcode == 0x00) {
                        QJsonDocument resp_doc = QJsonDocument::fromJson(payload);
                        if (!resp_doc.isNull()) {
                            QJsonObject resp = resp_doc.object();
                            QJsonObject resp_header = resp["header"].toObject();
                            int status = resp_header["status"].toInt();
                            int code = resp_header["code"].toInt();
                            QString message = resp_header["message"].toString();
                            
                            if (code != 0) {
                                StreamChunk chunk{message.toStdString(), true, 0.0f};
                                cb(chunk);
                                finished = true;
                                socket.close();
                                break;
                            }
                            
                            QJsonObject resp_payload = resp["payload"].toObject();
                            QJsonObject choices = resp_payload["choices"].toObject();
                            QJsonArray text_arr = choices["text"].toArray();
                            
                            for (const auto& item : text_arr) {
                                QString content = item.toObject()["content"].toString();
                                
                                // Stream character by character for better visual effect
                                for (int i = 0; i < content.length(); ) {
                                    // Handle UTF-8 multi-byte characters
                                    int char_len = 1;
                                    ushort unicode = content[i].unicode();
                                    if (unicode >= 0x4E00 && unicode <= 0x9FFF) {
                                        // Chinese character
                                        char_len = 1;
                                    }
                                    
                                    QString char_str = content.mid(i, char_len);
                                    StreamChunk chunk{char_str.toStdString(), false, 1.0f};
                                    cb(chunk);
                                    
                                    // Small delay for streaming effect
                                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                                    
                                    i += char_len;
                                }
                            }
                            
                            if (status == 2) {
                                StreamChunk chunk{"", true, 1.0f};
                                cb(chunk);
                                finished = true;
                                socket.close();
                            }
                        }
                    } else if (opcode == 0x08) {
                        finished = true;
                        socket.close();
                    }
                }
            }
        }
        
        timeout_count++;
    }
    
    if (!finished) {
        StreamChunk chunk{"Request timeout", true, 0.0f};
        cb(chunk);
        socket.close();
    }
}

std::string LLMClient::parseOpenAIResponse(const std::string& response) {
    // 简单的JSON解析（实际应用应用QJsonDocument）
    return response;
}

} // namespace ai
