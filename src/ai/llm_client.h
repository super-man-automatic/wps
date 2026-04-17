#pragma once
#include <string>
#include <functional>
#include <memory>
#include <future>
#include <QObject>

namespace ai {

struct StreamChunk {
    std::string text;
    bool is_finish = false;
    float confidence = 1.0f;
};

enum class Backend { OpenAI, LocalLLaMA, Xunfei, Mock };

class LLMClient : public QObject {
    Q_OBJECT
public:
    using StreamCallback = std::function<void(const StreamChunk&)>;

    explicit LLMClient(Backend backend = Backend::OpenAI, QObject* parent = nullptr);
    ~LLMClient();

    // 同步请求
    std::string complete(const std::string& prompt,
                        const std::string& model = "gpt-3.5-turbo");

    // 流式请求 - 使用 QtConcurrent 在后台线程运行
    void completeStream(const std::string& prompt,
                       StreamCallback on_chunk,
                       const std::string& model = "gpt-3.5-turbo");

    // 异步请求
    std::future<std::string> completeAsync(const std::string& prompt);

    // 配置
    void setApiKey(const std::string& key);
    void setApiSecret(const std::string& secret);
    void setAppId(const std::string& app_id);
    void setLocalModelPath(const std::string& path);
    void setBackend(Backend backend);
    void stop(); // Request to stop current generation

    bool isReady() const;

signals:
    void streamChunkReceived(const QString& chunk);
    void streamFinished();
    void errorOccurred(const QString& error);

private:
    Backend backend_;
    std::string api_key_;
    std::string api_secret_;
    std::string app_id_;
    std::string local_model_path_;
    QString session_id_; // Program-level session ID for context isolation
    std::atomic<bool> should_stop_{false}; // Flag to stop generation

    void requestOpenAI(const std::string& prompt, StreamCallback cb);
    void requestXunfei(const std::string& prompt, StreamCallback cb);
    void requestMock(const std::string& prompt, StreamCallback cb);
    std::string parseOpenAIResponse(const std::string& response);
};

} // namespace ai
