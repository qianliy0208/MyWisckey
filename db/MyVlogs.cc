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
            if(vlogs_[i]) {
                delete vlogs_[i];
            }
        }
        for(auto& e:read_files_) {
            fclose(e.second);
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
    void MyVlogs::Put(const std::string& key,const std::string& val,std::string* val_ptr) {
        // 获取sub_index
        int sub = GetSubIndex(key);

        // 获得对应的文件写入指针
        LogFile* logFile = GetVlogs(sub);
        //
        // logFile->Write1(key,val,val_ptr);
       logFile->Write(key,val,val_ptr);
    }
    // 讀取對應的文件獲取對應val
    void MyVlogs::Get(const std::string& val_ptr,std::string* val) {
        // 读取逻辑 ：獲取
        uint32_t fn;
        uint64_t pos;
        uint64_t len;
        Slice val_ptr_slice(val_ptr);
        GetVarint32(&val_ptr_slice,&fn);
        GetVarint64(&val_ptr_slice,&pos);
        GetVarint64(&val_ptr_slice,&len);

        //獲取文件名
        std::string file_path = std::string(c_log_path_) + std::to_string(fn);
        FILE * read_file = read_files_[file_path];
        if(!read_file) {
            // 打开文件
            FILE* read_file = fopen(file_path.c_str(), "r+");
            if (read_file == nullptr) {
                // Handle the error appropriately
                std::cerr << "Failed to open file: " << file_path << std::endl;
            }
            read_files_[file_path] = read_file;
        }
        // 讀取對應文件
        char buff[8096];  // 最多讀取 8KB
        fseek(read_file,pos,0);
        int read = fread(buff,1,len,read_file);

        val->assign(buff,read);

        //fclose(read_file);
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
        uint8_t index =  std::stoul(key) & 0x0F;// todo:需要根据子范围个数进行修改！

        return index;
    }

    uint64_t MyVlogs::GetNewFileNumber() {
        return file_num.fetch_add(1);
    }


}
