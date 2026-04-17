#include <gtest/gtest.h>
#include <memory>
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
