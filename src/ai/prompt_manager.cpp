#include "prompt_manager.h"

namespace ai {

PromptManager::PromptManager() {
    initDefaultPrompts();
}

void PromptManager::initDefaultPrompts() {
    templates_[static_cast<int>(Task::ContinueWriting)] =
        "请根据以下内容继续创意写作，语言保持{{language}}：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::Polish)] =
        "请润色以下{{language}}文本，使其更加流畅和有表现力：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::Translate)] =
        "请将以下文本翻译成{{language}}：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::Summarize)] =
        "请用{{language}}总结以下文本的核心要点，保持原意：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::Explain)] =
        "请用简单清晰的{{language}}语言解释以下内容：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::FixGrammar)] =
        "请检查并修正以下{{language}}文本的语法错误：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::GenerateOutline)] =
        "请为以下{{language}}文本生成一个详细的大纲：\n\n{{content}}\n\n";

    templates_[static_cast<int>(Task::CodeExplain)] =
        "请用{{language}}语言解释以下代码的功能和逻辑：\n\n{{content}}\n\n";
}

std::string PromptManager::build(Task task,
                                 const std::string& content,
                                 const std::string& context,
                                 const std::string& language) {
    int task_id = static_cast<int>(task);
    if (templates_.find(task_id) == templates_.end()) {
        return content;
    }

    std::string prompt = templates_[task_id];

    // 替换占位符
    size_t pos;
    while ((pos = prompt.find("{{content}}")) != std::string::npos) {
        prompt.replace(pos, 11, content);
    }
    while ((pos = prompt.find("{{context}}")) != std::string::npos) {
        prompt.replace(pos, 11, context);
    }
    while ((pos = prompt.find("{{language}}")) != std::string::npos) {
        prompt.replace(pos, 12, language);
    }

    return prompt;
}

void PromptManager::setCustomPrompt(Task task, const std::string& template_str) {
    templates_[static_cast<int>(task)] = template_str;
}

std::string PromptManager::taskToString(Task task) {
    switch (task) {
        case Task::ContinueWriting: return "ContinueWriting";
        case Task::Polish: return "Polish";
        case Task::Translate: return "Translate";
        case Task::Summarize: return "Summarize";
        case Task::Explain: return "Explain";
        case Task::FixGrammar: return "FixGrammar";
        case Task::GenerateOutline: return "GenerateOutline";
        case Task::CodeExplain: return "CodeExplain";
        default: return "Unknown";
    }
}

} // namespace ai
