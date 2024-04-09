#include "LogFiles.h"
#include <cstdint>  // For uint64_t
#include <iostream> // For error logging

namespace leveldb {
    const char* c_log_path_ = "/tmp/clog/"; // 冷日志文件目录
    const char* h_log_path_ = "/mnt/pmemdir/hlog/";  // 热日志文件目录

    LogFile::LogFile() {
        uint64_t file_num = MyVlogs::GetNewFileNumber();
        std::string file_path = std::string(c_log_path_) + std::to_string(file_num);
        // 打开文件
        dest_ = fopen(file_path.c_str(), "a+");
        if (dest_ == nullptr) {
            // Handle the error appropriately
            std::cerr << "Failed to open file: " << file_path << std::endl;
        }
        // 初始化文件指针
        file_offset_ = 0;
        block_buff_offset_ = 0;

    }

    void LogFile::FlushBlock() {
        // 刷新block块内容
        fwrite(dest_,)
    }

    void LogFile::WriteIntoBuff(const char* record,int size) {
        if(block_buff_offset_ + size > BLOCK_SIZE) {
            FlushBlock();
        }
    }
}
