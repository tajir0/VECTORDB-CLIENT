#include "vector_store.hpp"
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp> 
#include <mongocxx/uri.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

VectorStore::VectorStore(const std::string& uri, 
                        const std::string& db_name,
                        const std::string& collection_name) {
    mongocxx::instance instance{};
    try {
        // 初始化 MongoDB 客户端
        mongocxx::uri mongodb_uri{uri};
        client_ = mongocxx::client{mongodb_uri};
        
        // 尝试连接并获取集合
        auto db = client_[db_name];
        collection_ = db[collection_name];
        
        // 验证连接
        auto ping_cmd = bsoncxx::builder::stream::document{} << "ping" << 1 << finalize;
        db.run_command(ping_cmd.view());
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to connect to MongoDB: " + std::string(e.what()));
    }
    
    // 创建 2dsphere 索引用于向量搜索
    auto index_spec = document{} 
        << "embedding" << "2dsphere" << finalize;
    collection_.create_index(std::move(index_spec));
}

void VectorStore::insert(const VectorDoc& doc) {
    auto bson_doc = vectorDocToBson(doc);
    collection_.insert_one(bson_doc.view());
}

bsoncxx::document::value VectorStore::vectorDocToBson(const VectorDoc& doc) {
    // 构建BSON文档
    auto builder = document{};
    
    // 添加ID
    builder << "_id" << doc.id;
    
    // 添加向量数据
    auto array_builder = bsoncxx::builder::basic::array{};
    for (float val : doc.embedding) {
        array_builder.append(val);
    }
    builder << "embedding" << array_builder.view();
    
    // 添加元数据
    auto metadata_doc = document{};
    for (const auto& [key, value] : doc.metadata) {
        metadata_doc << key << value;
    }
    builder << "metadata" << metadata_doc;
    
    return builder << finalize;
}

VectorStore::VectorDoc VectorStore::bsonToVectorDoc(
    const bsoncxx::document::view& view) {
    VectorDoc doc;
    
    // 解析ID
    doc.id = view["_id"].get_string().value.to_string();
    
    // 解析向量
    auto embedding_array = view["embedding"].get_array().value;
    for (const auto& val : embedding_array) {
        doc.embedding.push_back(val.get_double());
    }
    
    // 解析元数据
    auto metadata_doc = view["metadata"].get_document().value;
    for (auto&& element : metadata_doc) {
        std::string key = element.key().to_string();
        std::string value = element.get_string().value.to_string();
        doc.metadata[key] = value;
    }
    
    return doc;
}

std::vector<VectorStore::VectorDoc> VectorStore::search(
    const std::vector<float>& query_vec, int top_k) {
    
    // 构建聚合管道进行向量搜索
    auto pipeline = mongocxx::pipeline{};
    
    // 使用 $vectorSearch 进行向量搜索
    pipeline.append_stage(document{} << "$vectorSearch" << open_document
        << "queryVector" << open_array
        << [&query_vec](bsoncxx::builder::stream::array_context<> arr) {
            for (float val : query_vec) arr << val;
        }
        << close_array
        << "k" << top_k
        << "path" << "embedding"
        << close_document << finalize);
        
    // 执行查询并收集结果
    auto cursor = collection_.aggregate(pipeline);
    
    std::vector<VectorDoc> results;
    for (auto&& doc : cursor) {
        results.push_back(bsonToVectorDoc(doc));
    }
    
    return results;
}

VectorStore::VectorDoc VectorStore::find(const std::string& id) {
    // 构建查询条件
    auto query_builder = document{} << "_id" << id << finalize;
    
    // 执行查询
    auto result = collection_.find_one(query_builder.view());
    
    // 转换结果为 VectorDoc
    if (result) {
        return bsonToVectorDoc(*result);
    }
    
    // 如果没找到，抛出异常
    throw std::runtime_error("Document not found: " + id);
}

void VectorStore::remove(const std::string& id) {
    // 构建删除条件
    auto delete_builder = document{} << "_id" << id << finalize;
    
    // 执行删除
    auto result = collection_.delete_one(delete_builder.view());
    
    // 检查是否删除成功
    if (!result || result->deleted_count() == 0) {
        throw std::runtime_error("Document not found: " + id);
    }
}

void VectorStore::batchInsert(const std::vector<VectorDoc>& docs) {
    std::vector<bsoncxx::document::value> documents;
    for (const auto& doc : docs) {
        documents.push_back(vectorDocToBson(doc));
    }
    
    auto result = collection_.insert_many(documents);
    if (!result) {
        throw std::runtime_error("Batch insert failed");
    }
}

void VectorStore::update(const std::string& id, const VectorDoc& doc) {
    // 构建更新条件和新文档
    auto filter = document{} << "_id" << id << finalize;
    auto update_doc = vectorDocToBson(doc);
    
    // 执行更新
    auto result = collection_.replace_one(filter.view(), update_doc.view());
    
    // 检查更新结果
    if (!result || result->modified_count() == 0) {
        throw std::runtime_error("Document not found: " + id);
    }
}