// HelloWorld.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <jni.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "miner.h"
#include "verusscan.h"
#include "equihash.h"
#include "stratum_client_socket.h"
#include <thread>
#include <vector>

#include "my_log.h"
//#include "aes_ni_arm_alias.h"

struct stratum_ctx stratum = { 0 };

std::atomic<bool> running(true);

void stratum_thread(StratumClient *client, stratum_ctx& stratum, work *g_work) {
    connect_stratum(*client, &stratum, g_work);
    running = false; // Ngắt thread mining nếu stratum ngừng
}

void mining_thread(StratumClient& client) {
    while (running) {
        if (g_job_queue.size() > 0) {
            work job = g_job_queue.getJob();
            std::cout << "\nmining {" << job.job_id << "}\n";
            LOGI("Mining %s", job.job_id);
            char buf[128];
            snprintf(buf, sizeof(buf), "Mining %s", job.job_id);
            add_status_line(buf);
            mining(job, client);
        }
    }
}

void start_mining() {
    stratum.is_equihash = true;

    struct work* g_work = new work();
    *g_work = { 0 };

    // Thông tin kết nối của Luckpool Stratum (cần kiểm tra lại thông số chính xác từ tài liệu của pool)
    std::string host = "ap.luckpool.net";  // Địa chỉ server
    int port = 3956; // Port kết nối (có thể thay đổi theo pool)

    StratumClient *client = new StratumClient(host, port);

    // Tạo 2 thread: 1 cho connect_stratum, 1 cho mining
    std::thread t1(stratum_thread, client, std::ref(stratum), g_work);
    std::thread t2(mining_thread, std::ref(*client));

    t1.detach();
    t2.detach();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myneon_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {

    std::string msg = get_status_string();

    return env->NewStringUTF(msg.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myneon_MainActivity_startMiningFromJNI(
        JNIEnv* env,
jobject /* this */
        ) {
    start_mining();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_myneon_MainActivity_updateDeviceIdFromJNI(
        JNIEnv* env,
        jobject /* this */,
        jint id
) {
    uint32_t read_id = static_cast<uint32_t>(id);
    updateDeviceId(read_id);
}