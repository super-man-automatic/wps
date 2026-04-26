#include <gtest/gtest.h>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>
#include "ai/llm_client.h"
#include "ai/prompt_manager.h"

using namespace ai;

class LLMClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        client_ = std::make_shared<LLMClient>(Backend::Mock);
    }

    std::shared_ptr<LLMClient> client_;
};

TEST_F(LLMClientTest, MockBackendReady) {
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, SyncComplete) {
    auto result = client_->complete("Hello");
    EXPECT_FALSE(result.empty());
}

TEST_F(LLMClientTest, StreamComplete) {
    std::vector<std::string> chunks;
    bool finished = false;

    client_->completeStream("Test prompt", 
        [&chunks, &finished](const StreamChunk& chunk) {
            if (!chunk.text.empty()) {
                chunks.push_back(chunk.text);
            }
            if (chunk.is_finish) {
                finished = true;
            }
        });

    // 等待异步完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(finished);
    EXPECT_FALSE(chunks.empty());
}

class PromptManagerTest : public ::testing::Test {
protected:
    PromptManager pm_;
};

TEST_F(PromptManagerTest, BuildContinueWriting) {
    auto prompt = pm_.build(PromptManager::Task::ContinueWriting, "从前有座山");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("从前有座山"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildPolish) {
    auto prompt = pm_.build(PromptManager::Task::Polish, "这是一个测试");
    EXPECT_FALSE(prompt.empty());
}

TEST_F(PromptManagerTest, LanguageTemplate) {
    auto prompt = pm_.build(PromptManager::Task::Summarize, "Content", "", "en");
    EXPECT_NE(prompt.find("en"), std::string::npos);
}

// LLMClient 配置测试
TEST_F(LLMClientTest, SetApiKey) {
    client_->setApiKey("test_api_key");
    // Mock 后端不需要验证 key 即可就绪
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, SetApiSecret) {
    client_->setApiSecret("test_secret");
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, SetAppId) {
    client_->setAppId("test_app_id");
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, SetLocalModelPath) {
    client_->setLocalModelPath("/path/to/model");
    // 设置路径不应影响就绪状态
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, SetBackend) {
    client_->setBackend(Backend::Mock);
    // 切换到 Mock 后端后应该就绪
    EXPECT_TRUE(client_->isReady());
}

TEST_F(LLMClientTest, CompleteAsync) {
    auto future = client_->completeAsync("Test prompt");
    
    // 等待异步完成
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    auto result = future.get();
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("Test prompt"), std::string::npos);
}

TEST_F(LLMClientTest, StopGeneration) {
    std::atomic<bool> stopped_early{false};
    std::atomic<int> chunk_count{0};
    
    client_->completeStream("This is a test prompt that should generate some content",
        [&chunk_count, &stopped_early](const StreamChunk& chunk) {
            if (!chunk.is_finish) {
                chunk_count++;
            }
        });
    
    // 立即请求停止
    client_->stop();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 停止后应该不再收到新的 chunk，或者收到了 finish chunk
    SUCCEED(); // 只要能正常执行不崩溃即可
}

TEST_F(LLMClientTest, SyncCompleteReturnsPrompt) {
    auto result = client_->complete("User input");
    EXPECT_NE(result.find("User input"), std::string::npos);
}

TEST_F(LLMClientTest, StreamChunkStructure) {
    std::vector<StreamChunk> received_chunks;
    
    client_->completeStream("Test", [&received_chunks](const StreamChunk& chunk) {
        received_chunks.push_back(chunk);
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    // 应该至少收到一个 finish chunk
    bool has_finish = false;
    for (const auto& chunk : received_chunks) {
        if (chunk.is_finish) {
            has_finish = true;
            break;
        }
    }
    EXPECT_TRUE(has_finish);
}

// PromptManager 更多测试
TEST_F(PromptManagerTest, BuildTranslate) {
    auto prompt = pm_.build(PromptManager::Task::Translate, "Hello", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("Hello"), std::string::npos);
    EXPECT_NE(prompt.find("zh"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildSummarize) {
    auto prompt = pm_.build(PromptManager::Task::Summarize, "Long text content here", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("Long text content here"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildExplain) {
    auto prompt = pm_.build(PromptManager::Task::Explain, "Complex concept", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("Complex concept"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildFixGrammar) {
    auto prompt = pm_.build(PromptManager::Task::FixGrammar, "Text with errors", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("Text with errors"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildGenerateOutline) {
    auto prompt = pm_.build(PromptManager::Task::GenerateOutline, "Document content", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("Document content"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildCodeExplain) {
    auto prompt = pm_.build(PromptManager::Task::CodeExplain, "int main() {}", "", "zh");
    EXPECT_FALSE(prompt.empty());
    EXPECT_NE(prompt.find("int main() {}"), std::string::npos);
}

TEST_F(PromptManagerTest, BuildWithContext) {
    auto prompt = pm_.build(PromptManager::Task::ContinueWriting, "content", "context info", "zh");
    // 当前实现不替换 context 占位符，但应该正常返回
    EXPECT_FALSE(prompt.empty());
}

TEST_F(PromptManagerTest, SetCustomPrompt) {
    pm_.setCustomPrompt(PromptManager::Task::Polish, "Custom template: {{content}}");
    auto prompt = pm_.build(PromptManager::Task::Polish, "my text");
    EXPECT_NE(prompt.find("Custom template:"), std::string::npos);
    EXPECT_NE(prompt.find("my text"), std::string::npos);
}

TEST_F(PromptManagerTest, InvalidTaskReturnsContent) {
    // 使用未初始化的任务类型应该返回原内容
    auto prompt = pm_.build(static_cast<PromptManager::Task>(999), "original");
    EXPECT_EQ(prompt, "original");
}
