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

std::string AIWriter::continueWritingPrompt(const std::string& prefix) {
    return prompt_manager_.build(PromptManager::Task::ContinueWriting, prefix);
}
std::string AIWriter::polish(const std::string& text) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Polish, text);
    return client_->complete(prompt);
}

std::string AIWriter::polishPrompt(const std::string& text) {
    return prompt_manager_.build(PromptManager::Task::Polish, text);
}
std::string AIWriter::translate(const std::string& text, const std::string& target_language) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Translate, text, "", target_language);
    return client_->complete(prompt);
}

std::string AIWriter::summarize(const std::string& text) {
    auto prompt = prompt_manager_.build(PromptManager::Task::Summarize, text);
    return client_->complete(prompt);
}

std::string AIWriter::summarizePrompt(const std::string& text) {
    return prompt_manager_.build(PromptManager::Task::Summarize, text);
}

std::string AIWriter::summarizeLongDoc(const std::vector<std::string>& chunks) {
    // Combine all chunks and summarize once
    std::string combined;
    for (const auto& chunk : chunks) {
        combined += chunk + "\n";
    }

    // 第二步：综合摘要
    auto final_prompt = prompt_manager_.build(PromptManager::Task::Summarize, combined);
    return client_->complete(final_prompt);
}

} // namespace ai
