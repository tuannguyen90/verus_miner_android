#ifndef GLOBALS_H
#define GLOBALS_h

#include <vector>
#include <thread>
#include <atomic>
#include "job_queue.h"

// Cho status_queue
#include <deque>
#include <mutex>
#include <string>

extern std::atomic<bool> stop_threads;

extern std::atomic<bool> mined;

extern std::vector<std::thread> workers;

extern std::atomic<int> active_threads;

extern std::atomic<uint32_t> mined_nonce_buf;

extern std::atomic<uint32_t> mined_nonce_throughput;

extern JobQueue g_job_queue;

extern std::atomic<uint32_t> deviceId;
void updateDeviceId(uint32_t deviceID);

// status_queue
extern std::deque<std::string> status_queue;
extern std::mutex status_mutex;

// Hàm thêm dòn status mới
void add_status_line(const std::string& line);

// Hàm lấy toàn bộ status nối thành 1 chuỗi
std::string get_status_string();



#endif