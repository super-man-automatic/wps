#include "xunfei_client.h"
#include <QWebSocket>
#include <QUrl>
#include <QDateTime>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QUuid>

namespace ai {

XunfeiClient::XunfeiClient(const XunfeiConfig& config, QObject* parent)
    : QObject(parent), config_(config), websocket_(nullptr) {
    websocket_ = new QWebSocket();
    connect(websocket_, &QWebSocket::textMessageReceived, this, [this](const QString& message) {
        parseResponse(message.toStdString());
    });
    connect(websocket_, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, [this](QAbstractSocket::SocketError error) {
        emit errorOccurred(websocket_->errorString());
    });
}

XunfeiClient::~XunfeiClient() {
    if (websocket_) {
        websocket_->close();
        websocket_->deleteLater();
    }
}

std::string XunfeiClient::generateSignature(const std::string& date) {
    // signature = base64(hmac_sha256(api_secret, host + "\n" + date))
    QString host = QUrl(QString::fromStdString(config_.ws_url)).host();
    QString signature_origin = QString("host: %1\ndate: %2\nGET /x2 HTTP/1.1")
        .arg(host, QString::fromStdString(date));
    
    QByteArray key = QByteArray::fromStdString(config_.api_secret);
    QByteArray message = signature_origin.toUtf8();
    
    QByteArray signature = QMessageAuthenticationCode::hash(
        message, key, QCryptographicHash::Sha256).toBase64();
    
    return signature.toStdString();
}

std::string XunfeiClient::generateAuthUrl() {
    QString host = QUrl(QString::fromStdString(config_.ws_url)).host();
    QString path = "/x2";
    
    // RFC1123 date format
    QString date = QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy HH:mm:ss") + " GMT";
    
    std::string signature = generateSignature(date.toStdString());
    
    // authorization = api_key + ":" + signature
    QString authorization = QString("%1:%2")
        .arg(QString::fromStdString(config_.api_key))
        .arg(QString::fromStdString(signature));
    
    QString authorization_base64 = authorization.toUtf8().toBase64();
    
    // Build URL with query parameters
    QUrlQuery query;
    query.addQueryItem("authorization", authorization_base64);
    query.addQueryItem("date", date);
    query.addQueryItem("host", host);
    
    QUrl url(QString::fromStdString(config_.ws_url) + "?" + query.toString());
    return url.toString().toStdString();
}

std::string XunfeiClient::buildRequestJson(const std::string& message) {
    QJsonObject header;
    header["app_id"] = QString::fromStdString(config_.app_id);
    header["uid"] = "user_" + QUuid::createUuid().toString(QUuid::IdOnly);
    
    QJsonObject parameter;
    QJsonObject chat;
    chat["domain"] = "general";
    chat["temperature"] = 0.5;
    chat["max_tokens"] = 4096;
    parameter["chat"] = chat;
    
    QJsonObject payload;
    QJsonObject text_obj;
    QJsonArray messages;
    QJsonObject user_msg;
    user_msg["role"] = "user";
    user_msg["content"] = QString::fromStdString(message);
    messages.append(user_msg);
    text_obj["text"] = messages;
    payload["message"] = text_obj;
    
    QJsonObject request;
    request["header"] = header;
    request["parameter"] = parameter;
    request["payload"] = payload;
    
    QJsonDocument doc(request);
    return doc.toJson(QJsonDocument::Compact).toStdString();
}

void XunfeiClient::chat(const std::string& message) {
    std::string auth_url = generateAuthUrl();
    websocket_->open(QUrl(QString::fromStdString(auth_url)));
    
    connect(websocket_, &QWebSocket::connected, this, [this, message]() {
        std::string json_request = buildRequestJson(message);
        websocket_->sendTextMessage(QString::fromStdString(json_request));
    });
}

void XunfeiClient::chatStream(const std::string& message,
                              std::function<void(const std::string&)> on_chunk,
                              std::function<void()> on_complete) {
    stream_callback_ = on_chunk;
    complete_callback_ = on_complete;
    accumulated_response_.clear();
    chat(message);
}

void XunfeiClient::parseResponse(const std::string& json) {
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json));
    QJsonObject obj = doc.object();
    
    QJsonObject header = obj["header"].toObject();
    int code = header["code"].toInt();
    
    if (code != 0) {
        QString error_msg = header["message"].toString();
        emit errorOccurred(error_msg);
        return;
    }
    
    QJsonObject payload = obj["payload"].toObject();
    QJsonObject choices = payload["choices"].toObject();
    QJsonArray text_array = choices["text"].toArray();
    
    QString content;
    for (const auto& item : text_array) {
        QJsonObject text_obj = item.toObject();
        content = text_obj["content"].toString();
        break;
    }
    
    int status = header["status"].toInt();
    
    if (stream_callback_) {
        stream_callback_(content.toStdString());
    }
    
    accumulated_response_ += content.toStdString();
    
    if (status == 2) {  // Complete
        emit responseReceived(QString::fromStdString(accumulated_response_));
        if (complete_callback_) {
            complete_callback_();
        }
        websocket_->close();
    }
}

} // namespace ai
