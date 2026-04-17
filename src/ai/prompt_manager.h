#pragma once
#include <string>
#include <unordered_map>

namespace ai {

class PromptManager {
public:
    enum class Task {
        ContinueWriting,
        Polish,
        Translate,
        Summarize,
        Explain,
        FixGrammar,
        GenerateOutline,
        CodeExplain
    };

    PromptManager();

    std::string build(Task task,
                     const std::string& content,
                     const std::string& context = "",
                     const std::string& language = "zh");

    void setCustomPrompt(Task task, const std::string& template_str);

private:
    std::unordered_map<int, std::string> templates_;
    void initDefaultPrompts();
    std::string taskToString(Task task);
};

} // namespace ai
