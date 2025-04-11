//// HelloWorld.cpp : This file contains the 'main' function. Program execution begins and ends there.
////
//#include <jni.h>
//#include <iostream>
//#include <iomanip>
//#include <cstdint>
//#include "miner.h"
//#include "verusscan.h"
//#include "equihash.h"
//#include "stratum_client_socket.h"
//#include <thread>
//#include <vector>
//
//
//struct stratum_ctx stratum = { 0 };
//
//std::atomic<bool> running(true);
//
//void stratum_thread(StratumClient& client, stratum_ctx& stratum, work* g_work) {
//    connect_stratum(client, &stratum, g_work);
//    running = false; // Ngắt thread mining nếu stratum ngừng
//}
//
//void mining_thread(StratumClient& client) {
//    while (running) {
//        if (g_job_queue.size() > 0) {
//            work job = g_job_queue.getJob();
//            std::cout << "\nmining {" << job.job_id << "}\n";
//            mining(job, client);
//        }
//    }
//}
//
////int main() {
////
////    /*unsigned char* msg = (unsigned char*)calloc(64, sizeof(unsigned char));
////    unsigned char* msg2 = (unsigned char*)calloc(32, sizeof(unsigned char));
////    unsigned char* digest = (unsigned char*)calloc(32, sizeof(unsigned char));
////
////    for (int i = 0; i < 64; ++i) {
////        msg[i] = i*4;
////    }
////
////    for (int i = 0; i < 32; ++i) {
////        msg2[i] = i;
////    }*/
////
////    /*load_constants();
////    haraka512(digest, msg);
////
////    printf("KQ:\n");
////    for (int i = 0; i < 32; ++i) {
////        printf("%02x ", digest[i]);
////    }
////    printf("\n");*/
////
////    /*load_constants();
////    haraka256(digest, msg2);
////    printf("KQ Haraka256:\n");
////    for (int i = 0; i < 32; ++i) {
////        printf("%02x ", digest[i]);
////    }
////    printf("\n");*/
////
////    /*printf("KQ Haraka512_keyed:\n");
////    load_constants();*/
////    /*for (int i = 0; i < 40; ++i) {
////        rc[i] = _mm_set_epi32(0x12121234, 0x5c4c3c5c, 0xff2c5daa, 0x5369d5c4);
////    }*/
////
////    //uint8_t bytes[16];
////    //_mm_storeu_si128((__m128i*)bytes, data_key);  // lưu vector ra mảng
////    //for (int i = 0; i < 16; ++i)
////    //    printf("%02X ", bytes[i]);
////
////    /*haraka512_keyed(digest, msg, rc);
////    printf("KQ:\n");
////    for (int i = 0; i < 32; ++i) {
////        printf("%02x ", digest[i]);
////    }
////    printf("\n");*/
////
////    //return 0;
////    stratum.is_equihash = true;
////
////    struct work g_work = { 0 };
////
////    // Thông tin kết nối của Luckpool Stratum (cần kiểm tra lại thông số chính xác từ tài liệu của pool)
////    std::string host = "ap.luckpool.net";  // Địa chỉ server
////    int port = 3960; // Port kết nối (có thể thay đổi theo pool)
////
////    StratumClient client(host, port);
////
////    // Tạo 2 thread: 1 cho connect_stratum, 1 cho mining
////    std::thread t1(stratum_thread, std::ref(client), std::ref(stratum), &g_work);
////    std::thread t2(mining_thread, std::ref(client));
////
////    // Đợi cả 2 thread kết thúc
////    t1.join();
////    t2.join();
////
////    return 0;
////}
//
//extern "C" JNIEXPORT jstring JNICALL
//Java_com_example_myneon_MainActivity_stringFromJNI(
//        JNIEnv* env,
//        jobject /* this */) {
////    std::string hello = "Hello from C++";
////    return env->NewStringUTF(hello.c_str());
//
//    stratum.is_equihash = true;
//
//    struct work g_work = { 0 };
//
//    // Thông tin kết nối của Luckpool Stratum (cần kiểm tra lại thông số chính xác từ tài liệu của pool)
//    std::string host = "ap.luckpool.net";  // Địa chỉ server
//    int port = 3960; // Port kết nối (có thể thay đổi theo pool)
//
//    StratumClient client(host, port);
//
//    // Tạo 2 thread: 1 cho connect_stratum, 1 cho mining
//    std::thread t1(stratum_thread, std::ref(client), std::ref(stratum), &g_work);
//    std::thread t2(mining_thread, std::ref(client));
//
//    // Đợi cả 2 thread kết thúc
//    t1.join();
//    t2.join();
//
//    std::string msg;
//
//#if defined(__ARM_NEON__)
//    msg = "__ARM_NEON__ detected";
//#elif defined(__ARM_NEON)
//    msg = "__ARM_NEON detected";
//#else
//    msg = "NEON not detected - using fallback";
//#endif
//
//    return env->NewStringUTF(msg.c_str());
//}