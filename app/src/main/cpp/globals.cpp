#include "globals.h"

#include "sstream"

std::deque<std::string> status_queue;
std::mutex status_mutex;

std::atomic<bool> stop_threads(false);

std::vector<std::thread> workers;

std::atomic<int> active_threads;

std::atomic<bool> mined(false);

std::atomic<uint32_t> mined_nonce_buf;

std::atomic<uint32_t> mined_nonce_throughput;

JobQueue g_job_queue;

std::atomic<uint32_t> deviceId(1);
void updateDeviceId(uint32_t deviceID) {
    ::deviceId.store(deviceID);
}

// Hàm thêm dòn status mới
void add_status_line(const std::string& line) {
    std::lock_guard<std::mutex> loc(status_mutex);
    if (status_queue.size() >= 10) {
        status_queue.pop_front();
    }
    status_queue.push_back(line);
}

// Hàm lấy toàn bộ status nối thành 1 chuỗi
std::string get_status_string() {
    std::lock_guard<std::mutex> lock(status_mutex);
    std::ostringstream oss;
    for (const auto& line : status_queue) {
        oss << line << "\n";
    }
    return  oss.str();
}