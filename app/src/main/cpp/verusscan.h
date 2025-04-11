#ifndef VERUSSCAN_H
#define VERUSSCAN_H

#include <stdint.h>
#include "uint256.h"
#include "verus_clhash.h"

// Nếu bạn có file `miner.h`, hãy include nó
#include "miner.h"
// Định nghĩa biến toàn cục (chỉ khai báo, không định nghĩa ở đây)
// extern uint32_t throughput;

// Khai báo các hàm cần sử dụng từ verusscan.cpp
#ifdef __cplusplus
extern "C"
{
#endif

    void FixKey(uint32_t* fixrand, uint32_t* fixrandex, u128* keyback, u128* g_prand, u128* g_prandex);
    void VerusHashHalf(void* result2, unsigned char* data, int len);
    void Verus2hash(unsigned char* hash, unsigned char* curBuf, unsigned char* nonce,
        u128* __restrict data_key, uint8_t* gpu_init, uint32_t* fixrand, uint32_t* fixrandex,
        u128* g_prand, u128* g_prandex, int version);

    //int scanhash_verus(int thr_id, struct work* work, uint32_t nonce, unsigned long* hashes_done);
    int scanhash_verus(int thr_id, struct work* work, uint32_t start_nonce, uint32_t max_nonce, unsigned long* hashes_done);

#ifdef __cplusplus
}
#endif

#endif // VERUSSCAN_H
