#include "document.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace smartdoc {

Document::Document(size_t chunk_size)
    : chunk_size_(chunk_size), total_chars_(0) {
    // 初始化第一个块
    chunks_.push_back(Chunk{0, 0, {}, true});
}

bool Document::loadFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

bool Document::loadFromString(const std::string& text) {
    std::unique_lock lock(mutex_);

    full_text_ = text;
    total_chars_ = text.size();

    // 清空旧块
    chunks_.clear();

    // 计算行数并分块
    size_t line_count = 0;
    size_t current_chunk = 0;

    chunks_.push_back(Chunk{0, 0, {}, true});

    for (char c : text) {
        if (c == '\n') {
            line_count++;
            if (line_count % chunk_size_ == 0) {
                chunks_[current_chunk].end_line = line_count;
                chunks_.push_back(Chunk{line_count, line_count, {}, false});
                current_chunk++;
            }
        }
    }

    if (!chunks_.empty()) {
        chunks_.back().end_line = line_count + 1;
    }

    return true;
}

bool Document::saveFile(const std::string& path) {
    std::shared_lock lock(mutex_);
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    file << full_text_;
    return file.good();
}

std::string Document::getText() const {
    std::shared_lock lock(mutex_);
    return full_text_;
}

std::string Document::getRange(size_t start, size_t end) const {
    std::shared_lock lock(mutex_);
    if (start >= full_text_.size()) return "";
    if (end > full_text_.size()) end = full_text_.size();
    return full_text_.substr(start, end - start);
}

void Document::modifyRange(size_t start, size_t end, const std::string& text) {
    std::unique_lock lock(mutex_);

    if (start >= full_text_.size()) return;
    if (end > full_text_.size()) end = full_text_.size();

    full_text_.replace(start, end - start, text);
    total_chars_ = full_text_.size();

    if (change_cb_) {
        lock.unlock();
        change_cb_(start, text.size());
    }
}

std::string Document::getContextAround(size_t pos, size_t context_lines) const {
    std::shared_lock lock(mutex_);

    // 简化版本：获取pos前后的行
    size_t start = 0;
    size_t end = full_text_.size();

    if (pos > 0) {
        // 向前找context_lines行
        int lines = 0;
        for (int i = pos - 1; i >= 0 && lines < static_cast<int>(context_lines); --i) {
            if (full_text_[i] == '\n') {
                lines++;
                start = i + 1;
            }
        }
    }

    // 向后找context_lines行
    {
        int lines = 0;
        for (size_t i = pos; i < full_text_.size() && lines < static_cast<int>(context_lines); ++i) {
            if (full_text_[i] == '\n') {
                lines++;
                end = i;
            }
        }
    }

    return full_text_.substr(start, end - start);
}

void Document::insertAISuggestion(size_t pos, const std::string& text) {
    modifyRange(pos, pos, text);
}

size_t Document::totalLines() const {
    std::shared_lock lock(mutex_);
    return std::count(full_text_.begin(), full_text_.end(), '\n') + 1;
}

void Document::ensureChunkLoaded(size_t chunk_idx) {
    std::unique_lock lock(mutex_);
    if (chunk_idx < chunks_.size() && !chunks_[chunk_idx].loaded) {
        // 实际加载逻辑（已简化）
        chunks_[chunk_idx].loaded = true;
    }
}

std::string Document::getLine(size_t line_num) const {
    std::shared_lock lock(mutex_);

    size_t current_line = 0;
    size_t start = 0;

    for (size_t i = 0; i < full_text_.size(); ++i) {
        if (current_line == line_num) {
            size_t end = full_text_.find('\n', i);
            if (end == std::string::npos) end = full_text_.size();
            return full_text_.substr(i, end - i);
        }
        if (full_text_[i] == '\n') {
            current_line++;
            start = i + 1;
        }
    }

    return "";
}

} // namespace smartdoc
