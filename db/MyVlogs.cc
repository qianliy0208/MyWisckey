//
// Created by jxx on 4/9/24.
//

#include "MyVlogs.h"
#include <assert.h>
namespace leveldb {
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
        // 放入逻辑
    }

    void MyVlogs::Get(const std::string& key,std::string* val) {
        // 读取逻辑
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
