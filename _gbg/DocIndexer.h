#pragma once
#include <string>
#include <vector>
#include <memory>
#include <ctime>

// ============================================================
//  CDocIndexer — 轻量级代码文件倒排索引引擎
//
//  用途：替代 xapian，为 CSolutionIndexer 提供
//        "基于 token 的倒排索引 + 全词匹配 FindInFiles" 能力。
//
//  核心设计：
//    · Tokenizer    : 按标识符/数字/符号分词，保留行号
//    · InvertedIndex: token(小写) → PostingList{ docId, lines[] }
//    · DocStore     : docId → { filePath, mtime }
//    · 持久化       : 单一二进制文件（.didx），带 magic + version
//    · 异步索引     : 内部工作线程消费任务队列，Find() 等待队列清空
// ============================================================

struct FindInFileResults;

class CDocIndexer
{
public:
    CDocIndexer();
    ~CDocIndexer();

    // ---- 生命周期 ----------------------------------------

    // 打开（或新建）索引库；indexPath 为存放 .didx 文件的目录
    bool Open(const char* indexPath);

    // 刷盘并关闭
    void Close();

    // ---- 文档管理（异步，立即返回）----------------------

    // 添加 / 更新一个文件（内容由调用方传入 UTF-8 字符串）
    void AddDocument(const std::string& filePath, time_t mtime, const std::string& utf8Content);

    // 删除一个文件
    void RemoveDocument(const std::string& filePath);

    // 查询某文件在索引中存储的 mtime（不存在则返回 0）
    time_t GetStoredMTime(const std::string& filePath);

    // 等待所有异步任务完成后刷盘（最多等待 timeoutMs 毫秒）
    // 返回 true 表示队列已空并刷盘成功
    bool FlushAndWait(int timeoutMs = 5000);

    // ---- 搜索 --------------------------------------------

    // 全词精确匹配搜索；会先等待队列清空（最多 500 ms）
    // key       : 搜索关键词（区分大小写）
    // maxResult : 最多返回的 FileInfo 条数
    bool Find(const char* key, int maxResult, FindInFileResults& results);

    // ---- 统计 --------------------------------------------
    size_t GetDocumentCount() const;

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};
