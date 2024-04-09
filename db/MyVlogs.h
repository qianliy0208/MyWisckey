//
// Created by jxx on 4/9/24.
//

#ifndef WISCKEY_SEPARATEKVSTORAGE_MASTER_MYVLOGS_H
#define WISCKEY_SEPARATEKVSTORAGE_MASTER_MYVLOGS_H

#include <vector>
#include <unordered_map>
#include <atomic>
#include "LogFiles.h"

namespace leveldb {

// 每个文件的元数据：
struct VFileMate {
    int num_r;  // 读取数
    int num_u;  // 更新数
};

class MyVlogs {

private:
    static std::atomic<uint64_t> file_num;  // 文件号管理
    int sub_num_;

    // 当前子范围指针 ：需要创建动态，删除
    std::vector<LogFile* > vlogs_;

    // 已经写入的vlog文件元数据 ,帮助我们读取搜索
    std::vector<std::unordered_map<uint64_t,VFileMate > > meta_files_;  // 需要创建删除



public:
    MyVlogs(int sub_num);
    virtual ~MyVlogs();

    LogFile* GetVlogs(int sub_index);
    void Put(const std::string& key,const std::string& val);
    void Get(const std::string& key,std::string* val);
    int GetSubIndex(const std::string& key);
    // 从这里获取文件号
    static uint64_t GetNewFileNumber();
};



};




#endif //WISCKEY_SEPARATEKVSTORAGE_MASTER_MYVLOGS_H
