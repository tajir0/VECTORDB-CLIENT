#include <gtest/gtest.h>
#include "../src/vector_store.hpp"

class VectorStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        try {
            store = std::make_unique<VectorStore>("mongodb://localhost:27017", "vectordb", "vectors");
        } catch (const std::exception& e) {
            mongodb_error = e.what();
        }
    }

    void TearDown() override {
        if (store) {
            try {
                store->remove("test_doc");
            } catch (...) {}
        }
    }

    std::unique_ptr<VectorStore> store;
    std::string mongodb_error;
};

// 测试数据库连接
TEST_F(VectorStoreTest, ConnectionTest) {
    try {
        VectorStore test_store("mongodb://localhost:27017", "vectordb", "vectors");
        SUCCEED() << "Successfully connected to MongoDB";
    } catch (const std::exception& e) {
        FAIL() << "Failed to connect to MongoDB: " << e.what();
    }
}


TEST_F(VectorStoreTest, Insert) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    VectorStore::VectorDoc doc{
        "test_doc",
        {1.0, 2.0, 3.0},
        {{"type", "test"}}
    };
    EXPECT_NO_THROW(store->insert(doc));
}

// 测试批量插入操作
TEST_F(VectorStoreTest, BatchInsert) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    std::vector<VectorStore::VectorDoc> docs = {
        {"test_doc1", {1.0, 2.0, 3.0}, {{"type", "test1"}}},
        {"test_doc2", {4.0, 5.0, 6.0}, {{"type", "test2"}}}
    };
    EXPECT_NO_THROW(store->batchInsert(docs));
}

// 测试查找操作
TEST_F(VectorStoreTest, Find) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    VectorStore::VectorDoc doc{
        "test_doc",
        {1.0, 2.0, 3.0},
        {{"type", "test"}}
    };
    store->insert(doc);
    
    auto found = store->find("test_doc");
    EXPECT_EQ(found.id, doc.id);
    EXPECT_EQ(found.embedding, doc.embedding);
    EXPECT_EQ(found.metadata, doc.metadata);
}

// 测试更新操作
TEST_F(VectorStoreTest, Update) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    VectorStore::VectorDoc doc{
        "test_doc",
        {1.0, 2.0, 3.0},
        {{"type", "test"}}
    };
    store->insert(doc);
    
    VectorStore::VectorDoc new_doc{
        "test_doc",
        {4.0, 5.0, 6.0},
        {{"type", "updated"}}
    };
    EXPECT_NO_THROW(store->update("test_doc", new_doc));
    
    auto found = store->find("test_doc");
    EXPECT_EQ(found.embedding, new_doc.embedding);
    EXPECT_EQ(found.metadata, new_doc.metadata);
}

// 测试删除操作
TEST_F(VectorStoreTest, Remove) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    VectorStore::VectorDoc doc{
        "test_doc",
        {1.0, 2.0, 3.0},
        {{"type", "test"}}
    };
    store->insert(doc);
    
    EXPECT_NO_THROW(store->remove("test_doc"));
    EXPECT_THROW(store->find("test_doc"), std::runtime_error);
}

// 测试向量搜索
TEST_F(VectorStoreTest, Search) {
    if (!store) {
        GTEST_SKIP() << "MongoDB connection failed: " << mongodb_error;
    }
    VectorStore::VectorDoc doc{
        "test_doc",
        {1.0, 2.0, 3.0},
        {{"type", "test"}}
    };
    store->insert(doc);
    
    auto results = store->search({1.1, 2.1, 3.1}, 5);
    EXPECT_GT(results.size(), 0);
    if (!results.empty()) {
        EXPECT_EQ(results[0].id, "test_doc");
    }
}