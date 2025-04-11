#ifndef STRATUM_CLIENT_H
#define STRATUM_CLIENT_H

//#include <winsock2.h>
//#include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>

#include <sstream>
#include <string>

#include <jansson.h>

#include "miner.h"
#include "util.h"

#include <vector>
#include <thread>

#include "globals.h"
#include "job_queue.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

//typedef int ssize_t;

class StratumClient {
public:
    StratumClient(const std::string& host, int port);
    ~StratumClient();

    // Kết nối tới máy chủ
    bool connectToServer();

    // Gửi dữ liệu (dạng string) tới máy chủ
    bool sendData(const std::string& data);

    // Nhận dữ liệu từ máy chủ (với buffer kích thước 4096 byte)
    std::string receiveData();

    // Gửi submit work
    bool submitWork(const std::string submitStr);

    // Gửi thông điệp subscribe theo giao thức stratum
    bool subscribe();

    // Gửi thông điệp authorize theo giao thức stratum
    bool authorize(const std::string& username, const std::string& password);

private:
    int sockfd;
    std::string host;
    int port;
};

int connect_stratum(StratumClient& client, stratum_ctx* sctx, work* work);

void mining(work& workX, StratumClient& client);

#endif