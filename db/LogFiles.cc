#include "LogFiles.h"
#include "MyVlogs.h"
#include <cstdint>  // For uint64_t
#include <iostream> // For error logging
#include <cstring>
#include <assert.h>
#include <queue>
#include <mutex>
#include <condition_variable>
namespace leveldb {
    extern std::queue<WriteBatch *> batch_queue;
    extern std::mutex batch_queue_mutex;
    extern std::condition_variable batch_queue_cv;
    const char* c_log_path_ = "/tmp/clog/"; // 冷日志文件目录
    const char* h_log_path_ = "/mnt/pmemdir/hlog/";  // 热日志文件目录

    LogFile::LogFile() {
        OpenNewFile();
    }
   void LogFile::OpenNewFile() {
       file_num_ = MyVlogs::GetNewFileNumber();
       std::string file_path = std::string(c_log_path_) + std::to_string(file_num_);
       // 打开文件
       dest_ = fopen(file_path.c_str(), "a+");
       if (dest_ == nullptr) {
           // Handle the error appropriately
           std::cerr << "Failed to open file: " << file_path << std::endl;
       }
       // 初始化文件初始状态
       file_offset_ = 0;
       block_buff_offset_ = 0;

       batch_ = new WriteBatch;
    }
    LogFile::~LogFile() {
        if (dest_ != nullptr) {
            fclose(dest_);
        }
    }
    void LogFile::FlushBlock() {
        // 刷新block块内容  todo: 在这边刷新时才算写入文件：这里可以从缓存中解析出对应的key指针：

        fwrite(block_buff,1,block_buff_offset_,dest_);
        file_offset_ += block_buff_offset_;
        block_buff_offset_ = 0;

        if(file_offset_ > MAX_FILE_SIZE){
            assert(dest_);
            fclose(dest_);
            OpenNewFile();
        }

        // 将batch写到一个双队列中，唤醒写入线程进行读取
        std::unique_lock<std::mutex> lock(batch_queue_mutex);
        batch_queue.push(batch_);
        batch_queue_cv.notify_one();

        // 为batch 创建一个新的batch
        batch_ = new WriteBatch;
    }
 // 写到block_buff 里面： [********  ]
    void LogFile::WriteIntoBuff(const char* record,int size) {
        uint32_t next_pos = block_buff_offset_ + size;
        if(next_pos > BLOCK_SIZE) {
            FlushBlock();
        }
        memcpy(block_buff + block_buff_offset_,record,size);
        block_buff_offset_ = next_pos;
    }
    // 到达写入缓存前
    void LogFile::Write(const std::string& key,const std::string& val) {
        // 思考写入缓存前需要做些什么？？？ 同步记录 文件号，偏移量
        // 写入文件的有 key_size + value_size  + key + value
        uint32_t ks = key.size();
        uint32_t vs = val.size();
        std::string new_v;                         //长度，文件号，位置
        PutVarint64(&new_v, vs);
        PutVarint32(&new_v, file_num_);
        PutVarint64(&new_v, file_offset_ + block_buff_offset_ + 8 + ks);

        // 获取后将 key val 写入batch_中
        batch_->Put(key,new_v);

        // 放入缓存格式：ks vs  注意编码方式
        char head_buff[8];
        memcpy(head_buff,(char*)&ks,4);
        memcpy(head_buff + 4,(char*)&vs,4);

        // 放入缓存
        WriteIntoBuff(head_buff,8);
        WriteIntoBuff(key.data(),key.size());
        WriteIntoBuff(val.data(),val.size());

    }



}
