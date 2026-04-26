#include <gtest/gtest.h>
#include <memory>
#include <fstream>
#include <thread>
#include <vector>
#include "core/document.h"

using namespace smartdoc;

class DocumentTest : public ::testing::Test {
protected:
    void SetUp() override {
        doc_ = std::make_shared<Document>();
    }

    std::shared_ptr<Document> doc_;
};

TEST_F(DocumentTest, LoadFromString) {
    std::string content = "# Title\n\nParagraph 1\n\nParagraph 2";
    ASSERT_TRUE(doc_->loadFromString(content));
    EXPECT_EQ(doc_->getText(), content);
}

TEST_F(DocumentTest, TotalLines) {
    doc_->loadFromString("Line 1\nLine 2\nLine 3");
    EXPECT_EQ(doc_->totalLines(), 3);
}

TEST_F(DocumentTest, GetRange) {
    doc_->loadFromString("0123456789");
    EXPECT_EQ(doc_->getRange(0, 5), "01234");
    EXPECT_EQ(doc_->getRange(5, 10), "56789");
}

TEST_F(DocumentTest, ModifyRange) {
    doc_->loadFromString("Hello World");
    doc_->modifyRange(6, 11, "SmartDoc");
    EXPECT_EQ(doc_->getText(), "Hello SmartDoc");
}

TEST_F(DocumentTest, ThreadSafety) {
    doc_->loadFromString("Initial content");
    
    // 多线程读
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([this]() {
            for (int j = 0; j < 10; ++j) {
                auto text = doc_->getText();
                EXPECT_FALSE(text.empty());
            }
        });
    }

    // 多线程写
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([this, i]() {
            doc_->modifyRange(0, 7, "Test" + std::to_string(i));
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_GT(doc_->totalChars(), 0);
}

TEST_F(DocumentTest, TotalChars) {
    std::string content = "Hello World";
    doc_->loadFromString(content);
    EXPECT_EQ(doc_->totalChars(), content.size());
}

TEST_F(DocumentTest, IsPartialLoaded) {
    // 小文本不应被分块
    doc_->loadFromString("Small text");
    EXPECT_FALSE(doc_->isPartialLoaded());
}

TEST_F(DocumentTest, GetContextAround) {
    std::string content = "Line 1\nLine 2\nLine 3\nLine 4\nLine 5";
    doc_->loadFromString(content);
    
    // 测试获取位置周围的上下文（大约在Line 3的位置）
    std::string context = doc_->getContextAround(15, 1);
    EXPECT_FALSE(context.empty());
}

TEST_F(DocumentTest, InsertAISuggestion) {
    doc_->loadFromString("Hello ");
    doc_->insertAISuggestion(6, "SmartDoc");
    EXPECT_EQ(doc_->getText(), "Hello SmartDoc");
}

TEST_F(DocumentTest, GetRangeEdgeCases) {
    doc_->loadFromString("0123456789");
    
    // 起始位置越界
    EXPECT_EQ(doc_->getRange(100, 110), "");
    
    // 结束位置超过文本长度
    EXPECT_EQ(doc_->getRange(5, 100), "56789");
}

TEST_F(DocumentTest, ModifyRangeEdgeCases) {
    doc_->loadFromString("Hello World");
    
    // 起始位置越界不应修改
    doc_->modifyRange(100, 110, "Test");
    EXPECT_EQ(doc_->getText(), "Hello World");
    
    // 结束位置超过文本长度应被截断
    doc_->modifyRange(6, 100, "SmartDoc");
    EXPECT_EQ(doc_->getText(), "Hello SmartDoc");
}

TEST_F(DocumentTest, ChangeCallback) {
    bool callback_invoked = false;
    size_t callback_pos = 0;
    size_t callback_len = 0;
    
    doc_->onChanged([&](size_t pos, size_t len) {
        callback_invoked = true;
        callback_pos = pos;
        callback_len = len;
    });
    
    doc_->loadFromString("Initial");
    doc_->modifyRange(0, 7, "Modified");
    
    EXPECT_TRUE(callback_invoked);
    EXPECT_EQ(callback_pos, 0);
    EXPECT_EQ(callback_len, 8); // "Modified"的长度
}

TEST_F(DocumentTest, SaveAndLoadFile) {
    std::string test_file = "test_document_temp.txt";
    std::string content = "Test content for file operations\nLine 2\nLine 3";
    
    // 加载字符串并保存到文件
    doc_->loadFromString(content);
    EXPECT_TRUE(doc_->saveFile(test_file));
    
    // 创建新文档并从文件加载
    auto doc2 = std::make_shared<Document>();
    EXPECT_TRUE(doc2->loadFile(test_file));
    EXPECT_EQ(doc2->getText(), content);
    
    // 清理临时文件
    std::remove(test_file.c_str());
}

TEST_F(DocumentTest, LoadFileNotFound) {
    EXPECT_FALSE(doc_->loadFile("non_existent_file_12345.txt"));
}

TEST_F(DocumentTest, TotalLinesWithEmptyContent) {
    doc_->loadFromString("");
    // 空内容应该只有1行
    EXPECT_EQ(doc_->totalLines(), 1);
}

TEST_F(DocumentTest, TotalLinesMultiple) {
    doc_->loadFromString("Line1\nLine2\nLine3\nLine4");
    EXPECT_EQ(doc_->totalLines(), 4);
}

TEST_F(DocumentTest, EmptyDocument) {
    // 测试空文档行为
    EXPECT_EQ(doc_->totalChars(), 0);
    EXPECT_EQ(doc_->totalLines(), 1);
    EXPECT_EQ(doc_->getText(), "");
}
