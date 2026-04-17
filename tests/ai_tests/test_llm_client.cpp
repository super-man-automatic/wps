#include <gtest/gtest.h>
#include <memory>
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
