#pragma once
#include "llm_client.h"
#include "prompt_manager.h"
#include <memory>
#include <string>

namespace ai {

class AIWriter {
public:
    explicit AIWriter(std::shared_ptr<LLMClient> client);

    // 续写
    void continueWriting(const std::string& prefix,
                        LLMClient::StreamCallback on_stream);

    // 润色
    std::string polish(const std::string& text);

    // 翻译
    std::string translate(const std::string& text, const std::string& target_language);

    // 摘要
    std::string summarize(const std::string& text);

    // 长文档分段摘要
    std::string summarizeLongDoc(const std::vector<std::string>& chunks);

private:
    std::shared_ptr<LLMClient> client_;
    PromptManager prompt_manager_;
};

} // namespace ai
