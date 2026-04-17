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
<<<<<<< HEAD
    std::string continueWritingPrompt(const std::string& prefix);

    // 润色
    std::string polish(const std::string& text);
    std::string polishPrompt(const std::string& text);
=======

    // 润色
    std::string polish(const std::string& text);
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd

    // 翻译
    std::string translate(const std::string& text, const std::string& target_language);

    // 摘要
    std::string summarize(const std::string& text);
<<<<<<< HEAD
    std::string summarizePrompt(const std::string& text);
=======
>>>>>>> 5475e174cf498978befe4c9d3f475f4a1cc6c9bd

    // 长文档分段摘要
    std::string summarizeLongDoc(const std::vector<std::string>& chunks);

private:
    std::shared_ptr<LLMClient> client_;
    PromptManager prompt_manager_;
};

} // namespace ai
