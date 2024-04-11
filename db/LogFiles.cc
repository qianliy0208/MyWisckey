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

   //    batch_ = new WriteBatch;
    }
    void LogFile::OpenNewFile1() {
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

        //    batch_ = new WriteBatch;
    }
    LogFile::~LogFile() {
        if (dest_ != nullptr) {
            fclose(dest_);
        }
    }
  /*  void LogFile::FlushBlock() {
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
    }*/
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
    // 直接封裝後寫入文件,返回新的指針
    void LogFile::Write(const std::string& key,const std::string& val,std::string* new_v1) {
        // 思考写入缓存前需要做些什么？？？ 同步记录 文件号，偏移量
        // 写入文件的有 key_size + value_size  + key + value // value_size = 0  delete
        uint32_t ks = key.size();
        uint32_t vs = val.size();

        // 獲得寫入的信息；
        std::string new_v;                         //文件号，位置,长度，
        PutVarint32(&new_v, file_num_);
        PutVarint64(&new_v, file_offset_  + 8 + ks);
        PutVarint64(&new_v, vs);
        new_v1->assign(new_v);
        // 存放記錄
        char record[8192];    // 最大寫入八key
        // 拷貝到緩存
        char* end = record;
        memcpy(end,(char*)&ks,4);
        memcpy(end + 4,(char*)&vs,4);
        memcpy(end + 8,key.data(),ks);
        memcpy(end + 8 + ks,val.data(),vs);
        size_t len = 8 + ks + vs;

        // 寫入文件
        fwrite(record,1,len,dest_);

        // 更新文件偏移量
        file_offset_ += len;

        // 如果文件大於最大值，則打開新文件
        if(file_offset_ > MAX_FILE_SIZE){
            //assert(dest_);
            fclose(dest_);
            OpenNewFile();
        }
    }


    void LogFile::Write1(const std::string& key,const std::string& val,std::string* new_v) {
        // 思考写入缓存前需要做些什么？？？ 同步记录 文件号，偏移量
        // 写入文件的有 key_size + value_size  + key + value // value_size = 0  delete
        uint64_t b = clock();
        uint32_t ks = key.size();
        uint32_t vs = val.size();

        //长度，文件号，位置
        PutVarint64(new_v, vs);
        PutVarint32(new_v, file_num_);
        PutVarint64(new_v, file_offset_ + block_buff_offset_ + 8 + ks);
        uint64_t e1;
        e1 = clock();
        // 获取后将 key val 写入batch_中
       // batch_->Put(key,new_v);


        // 寫入緩存
        uint32_t next_pos = block_buff_offset_ + 8 + ks + vs;
        if(next_pos > BLOCK_SIZE) {
            uint64_t e3;
            e3 = clock();
            FlushBlock();
           // std::cout << clock() - e3 << std::endl;
        }
        char* end = block_buff + block_buff_offset_;
        memcpy(end,&ks,4);
        end = end + 4;
        memcpy(end,&vs,4);
        end = end + 4;
        memcpy(end,key.data(),ks);
        end = end + ks;
        memcpy(end,val.data(),vs);
        block_buff_offset_ = next_pos;
        //std::cout << (clock() - e1) << std::endl;
    }

void LogFile::Write(const std::string& key,const std::string& val) {
        // 思考写入缓存前需要做些什么？？？ 同步记录 文件号，偏移量
        // 写入文件的有 key_size + value_size  + key + value // value_size = 0  delete
        uint32_t ks = key.size();
        uint32_t vs = val.size();
        std::string new_v;                         //长度，文件号，位置
        PutVarint64(&new_v, vs);
        PutVarint32(&new_v, file_num_);
        uint64_t total_size = block_buff_offset_ + 8 + ks;
        PutVarint64(&new_v, file_offset_ + total_size);
        total_size += vs;

        // 获取后将 key val 写入batch_中
        batch_->Put(key,new_v);

        // 寫入緩存
        uint32_t next_pos = block_buff_offset_ + total_size;
        if(next_pos > BLOCK_SIZE) {
            FlushBlock();
        }
        char* end = block_buff + block_buff_offset_;
        memcpy(end,&ks,4);
        end = end + 4;
        memcpy(end,&vs,4);
        end = end + 4;
        memcpy(end,key.data(),ks);
        end = end + ks;
        memcpy(end,val.data(),vs);

        block_buff_offset_ = next_pos;


    }


}
