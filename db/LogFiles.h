//
// Created by jxx on 4/9/24.
//

#ifndef WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
#define WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
#include <string>
#include <stdio.h>
#include "write_batch_internal.h"
namespace leveldb{

    const uint64_t MAX_FILE_SIZE = 1024 * 1024 * 1024;
    const int BLOCK_SIZE = 1024 * 1024;  // 1MB
    class LogFile {
    private:
      //  MyVlogs* myVlogs;
        FILE* dest_;
        int32_t file_num_;
        uint64_t file_offset_;
        uint32_t block_buff_offset_;
        char block_buff[BLOCK_SIZE];  // 会被阻塞吗？？ 双缓存？？
        // 每次写一个key就会将其写入这个batch中，将新的key和对应的value：文件号——偏移——长度。之后将其返回给写入队列

        WriteBatch* batch_;

    public:

       LogFile();
       void OpenNewFile();
       void FlushBlock();
       virtual ~LogFile();
       void WriteIntoBuff(const char *record, int size);

       void Write(const std::string& key,const std::string& val);

    };
}
#endif //WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
