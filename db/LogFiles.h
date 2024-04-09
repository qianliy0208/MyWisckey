//
// Created by jxx on 4/9/24.
//

#ifndef WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
#define WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
#include <string>
#include <stdio.h>

namespace leveldb{

const int BLOCK_SIZE = 1024*1024;  // 1MB
class LogFile {
private:

    FILE* dest_;
    uint64_t file_offset_;
    uint32_t block_buff_offset_;
    char block_buff[BLOCK_SIZE];

public:

    LogFile();

    void WriteIntoBuff(const char* record);


};
}
#endif //WISCKEY_SEPARATEKVSTORAGE_MASTER_LOGFILES_H
