#pragma once
#include <string>
#include <vector>
#include <shared_mutex>
#include <memory>
#include <functional>

namespace smartdoc {

struct TextBlock {
    enum Type { Paragraph, Heading, Code, List, Table, Formula };
    Type type;
    std::string content;
    int level;
    std::vector<std::string> meta;
};

class Document : public std::enable_shared_from_this<Document> {
public:
    using ChangeCallback = std::function<void(size_t pos, size_t len)>;

    struct Chunk {
        size_t start_line;
        size_t end_line;
        std::vector<TextBlock> blocks;
        bool loaded = false;
    };

    explicit Document(size_t chunk_size = 1000);
    ~Document() = default;

    // 文件操作
    bool loadFile(const std::string& path);
    bool loadFromString(const std::string& text);
    bool saveFile(const std::string& path);

    // 文本操作（线程安全）
    std::string getRange(size_t start, size_t end) const;
    void modifyRange(size_t start, size_t end, const std::string& text);
    std::string getText() const;

    // AI相关
    std::string getContextAround(size_t pos, size_t context_lines = 10) const;
    void insertAISuggestion(size_t pos, const std::string& text);

    // 监听变更
    void onChanged(ChangeCallback cb) { change_cb_ = cb; }

    // 查询
    size_t totalLines() const;
    size_t totalChars() const { return total_chars_; }
    bool isPartialLoaded() const { return chunks_.size() > 1; }

private:
    mutable std::shared_mutex mutex_;
    std::vector<Chunk> chunks_;
    std::string full_text_;
    size_t chunk_size_;
    size_t total_chars_ = 0;
    ChangeCallback change_cb_;

    void ensureChunkLoaded(size_t chunk_idx);
    void rebuildIndex();
    std::string getLine(size_t line_num) const;
};

} // namespace smartdoc
