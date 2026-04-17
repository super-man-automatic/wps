#include "ai_writer.h"

namespace ai {

AIWriter::AIWriter(std::shared_ptr<LLMClient> client)
    : client_(client) {
}

void AIWriter::continueWriting(const std::string& prefix,
                              LLMClient::StreamCallback on_stream) {
    auto prompt = prompt_manager_.build(PromptManager::Task::ContinueWriting, prefix);
    client_->completeStream(prompt, on_stream);
}

std::string AIWriter::polish(const std::string& text) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Polish, text);
    return client_->complete(prompt);
}

std::string AIWriter::translate(const std::string& text, const std::string& target_language) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Translate, text, "", target_language);
    return client_->complete(prompt);
}

std::string AIWriter::summarize(const std::string& text) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Summarize, text);
    return client_->complete(prompt);
}

std::string AIWriter::summarizeLongDoc(const std::vector<std::string>& chunks) {
    // 第一步：每段摘要
    std::vector<std::string> partial_summaries;
    for (const auto& chunk : chunks) {
        auto prompt = prompt_manager_.build(PromptManager::Task::Summarize, chunk);
        partial_summaries.push_back(client_->complete(prompt));
    }

    // 第二步：综合摘要
    std::string combined;
    for (const auto& s : partial_summaries) {
        combined += s + "\n";
    }

    auto final_prompt = prompt_manager_.build(PromptManager::Task::Summarize, combined);
    return client_->complete(final_prompt);
}

} // namespace ai
