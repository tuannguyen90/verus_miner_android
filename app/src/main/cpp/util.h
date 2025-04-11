#pragma once
#include <cmath>
#include "miner.h"
#include "stdio.h"
#include "time.h"
#include <string>
#include "stratum_client_socket.h"

bool hex2bin(void* output, const char* hexstr, size_t len);
char* bin2hex(const uchar* in, size_t len);
void cbin2hex(char* out, const char* in, size_t len);
void* aligned_calloc(int size);

uint32_t getblocheight(struct stratum_ctx* sctx);
bool stratum_notify(struct stratum_ctx* sctx, json_t* params);

bool verus_submit(struct work* work, char** submitStr);

bool stratum_parse_extranonce(struct stratum_ctx* sctx, json_t* params, int pndx);