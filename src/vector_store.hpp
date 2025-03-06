// vector_store.hpp
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/document.hpp>

class VectorStore {
public:
    // 向量文档结构
    struct VectorDoc {
        std::string id;                              // 文档唯一标识符
        std::vector<float> embedding;                // 向量数据
        std::map<std::string, std::string> metadata; // 元数据
    };

    // 构造函数
    VectorStore(const std::string& uri,              // MongoDB 连接 URI
                const std::string& db_name,          // 数据库名称
                const std::string& collection_name);  // 集合名称

    // 基本CRUD操作
    void insert(const VectorDoc& doc);               // 插入单个文档
    void batchInsert(const std::vector<VectorDoc>& docs); // 批量插入文档
    VectorDoc find(const std::string& id);          // 根据ID查找文档
    void update(const std::string& id, const VectorDoc& doc); // 更新文档
    void remove(const std::string& id);             // 删除文档

    // 向量搜索
    std::vector<VectorDoc> search(const std::vector<float>& query_vec, 
                                 int top_k);         // 向量相似度搜索
    //根据id查找向量文档

private:
    mongocxx::client client_;                       // MongoDB 客户端
    mongocxx::collection collection_;                // MongoDB 集合
    
    // BSON转换辅助函数
    bsoncxx::document::value vectorDocToBson(const VectorDoc& doc);    // 文档转BSON
    VectorDoc bsonToVectorDoc(const bsoncxx::document::view& view);    // BSON转文档
};