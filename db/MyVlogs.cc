//
// Created by jxx on 4/9/24.
//

#include "MyVlogs.h"
#include <assert.h>
#include <iostream>
namespace leveldb {

   extern const char* c_log_path_ ; // 冷日志文件目录
   extern const char* h_log_path_ ;  // 热日志文件目录
    std::atomic<uint64_t> MyVlogs::file_num{0};
    MyVlogs::MyVlogs(int sub_num):sub_num_(sub_num) {
        // 初始化新的子vlog
        vlogs_.reserve(sub_num_);  // sub_num 初始化为256 8bit
        for(int i = 0;i < sub_num_;i++) {
            vlogs_[i] = new LogFile();
        }
    }
    MyVlogs::~MyVlogs() {
        for(int i = 0;i < sub_num_;i++) {
            delete vlogs_[i];
        }
    }
    LogFile* MyVlogs::GetVlogs(int sub_index) {
        return vlogs_[sub_index];
    }

    void MyVlogs::Put(const std::string& key,const std::string& val) {
        // 获取sub_index
        int sub = GetSubIndex(key);

        // 获得对应的文件写入指针
        LogFile* logFile = GetVlogs(sub);
        //

        logFile->Write(key,val);
    }

    void MyVlogs::Get(const std::string& key,std::string* val) {
        // 读取逻辑 ：可以直接调用原来的读取过程，先获取指针，之后读取文件
    }
    void MyVlogs::Get(char* buff,uint32_t file_num,uint64_t pos,uint64_t size,int sub) {
        std::string file_path = std::string(c_log_path_) + std::to_string(file_num);
        FILE* dest_ = fopen(file_path.c_str(), "a+");
        if (dest_ == nullptr) {
            // Handle the error appropriately
            std::cerr << "Failed to open file: " << file_path << std::endl;
        }
        fseek(dest_,pos,0);
        fread(buff,1,size,dest_);

    }
    int MyVlogs::GetSubIndex(const std::string& key) {
        assert(!key.empty());
        uint8_t index =  reinterpret_cast<char>(key[0]);// todo:需要根据子范围个数进行修改！
        return index;
    }

    uint64_t MyVlogs::GetNewFileNumber() {
        return file_num.fetch_add(1);
    }


}
