#include "llm_client.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <thread>

namespace ai {

LLMClient::LLMClient(Backend backend, QObject* parent)
    : QObject(parent), backend_(backend) {
}

LLMClient::~LLMClient() {
}

void LLMClient::setApiKey(const std::string& key) {
    api_key_ = key;
}

void LLMClient::setLocalModelPath(const std::string& path) {
    local_model_path_ = path;
}

void LLMClient::setBackend(Backend backend) {
    backend_ = backend;
}

bool LLMClient::isReady() const {
    if (backend_ == Backend::OpenAI) {
        return !api_key_.empty();
    } else if (backend_ == Backend::LocalLLaMA) {
        return !local_model_path_.empty();
    }
    return true;  // Mock always ready
}

std::string LLMClient::complete(const std::string& prompt, const std::string& model) {
    // 简化同步版本
    if (backend_ == Backend::Mock) {
        return "Mock response to: " + prompt;
    }

    // TODO: 实现真实的HTTP请求
    return "";
}

void LLMClient::completeStream(const std::string& prompt,
                               StreamCallback on_chunk,
                               const std::string& model) {
    // 在后台线程运行以避免阻塞
    std::thread([this, prompt, on_chunk, model]() {
        if (backend_ == Backend::OpenAI) {
            requestOpenAI(prompt, on_chunk);
        } else if (backend_ == Backend::Mock) {
            requestMock(prompt, on_chunk);
        }
    }).detach();
}

std::future<std::string> LLMClient::completeAsync(const std::string& prompt) {
    return std::async(std::launch::async, [this, prompt]() {
        return complete(prompt);
    });
}

void LLMClient::requestOpenAI(const std::string& prompt, StreamCallback cb) {
    // TODO: 实现OpenAI API调用
    StreamChunk chunk{"[API Key not configured]", true, 0.0f};
    cb(chunk);
}

void LLMClient::requestMock(const std::string& prompt, StreamCallback cb) {
    // 模拟流式响应
    std::string mock_response = "这是一个模拟AI响应。您输入的prompt是: " + prompt;

    // 按字符流式发送
    for (char c : mock_response) {
        StreamChunk chunk{std::string(1, c), false, 1.0f};
        cb(chunk);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 发送结束信号
    StreamChunk finish_chunk{"", true, 1.0f};
    cb(finish_chunk);
}

std::string LLMClient::parseOpenAIResponse(const std::string& response) {
    // 简单的JSON解析（实际应用应用QJsonDocument）
    return response;
}

} // namespace ai
