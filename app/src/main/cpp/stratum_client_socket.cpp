#include "stratum_client_socket.h"
#include "my_log.h"
#define EQNONCE_OFFSET 30
#define LP_SCANTIME		60

StratumClient::StratumClient(const std::string& host, int port) : host(host), port(port), sockfd(-1) {}

StratumClient::~StratumClient() {
    if (sockfd != -1) {
//        closesocket(sockfd);
close(sockfd);
    }
}

// Kết nối tới máy chủ
bool StratumClient::connectToServer() {
    struct addrinfo hints, * res, * p;
    int status;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    LOGI("Ket noi server");
    if ((status = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res)) != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        LOGE("getaddrinfo error: %s", gai_strerror(status));
        return false;
    }

    // Duyệt qua các địa chỉ trả về và thử kết nối
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
//            closesocket(sockfd);
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break; // Kết nối thành công
    }

    freeaddrinfo(res);

    if (sockfd == -1) {
        std::cerr << "Connecting failed " << host << " on port " << port << std::endl;
        LOGI("Ket noi server failed (line 52)");
        return false;
    }
    std::cout << "Connected " << host << " on port " << port << std::endl;
    LOGI("Connected on port %d", port);
    add_status_line("Connected");
    return true;
}

// Gửi dữ liệu (dạng string) tới máy chủ
bool StratumClient::sendData(const std::string& data) {
    ssize_t totalSent = 0;
    ssize_t dataLen = data.size();
    while (totalSent < dataLen) {
        ssize_t sent = send(sockfd, data.c_str() + totalSent, dataLen - totalSent, 0);
        if (sent == -1) {
            std::cerr << "Send data failed" << std::endl;
            return false;
        }
        totalSent += sent;
    }
    return true;
}

// Nhận dữ liệu từ máy chủ (với buffer kích thước 4096 byte)
std::string StratumClient::receiveData() {
    char buffer[4096];
    std::string received;
    ssize_t bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        received = buffer;
    }

    return received;
}

// Gửi submit work
bool StratumClient::submitWork(const std::string submitStr) {
    if (!sendData(submitStr)) {
        std::cerr << "Sent submit failed" << std::endl;
        LOGI("Sent submit failed");
    }

    /*std::string response = receiveData();
    std::cout << "Submit Reponse: " << response << std::endl;*/

    return true;
}

// Gửi thông điệp subscribe theo giao thức stratum
bool StratumClient::subscribe() {
    // Thông điệp subscribe mẫu
    // id: 1, method: mining.subscribe, params: []
    std::string subscribeMsg = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n";
    if (!sendData(subscribeMsg)) {
        std::cerr << "Sent subcribe failed" << std::endl;
        LOGI("Sent subcribe failed");
        return false;
    }
    //std::cout << "Sent subscribe: " << subscribeMsg;
    // Nhận phản hồi từ server
    /*std::string response = receiveData();
    std::cout << "subscribe Reponse: " << response << std::endl;*/
    return true;
}

// Gửi thông điệp authorize theo giao thức stratum
bool StratumClient::authorize(const std::string& username, const std::string& password) {
    // Thông điệp authorize mẫu
    // id: 2, method: mining.authorize, params: [username, password]
    std::string authorizeMsg = "{\"id\": 2, \"method\": \"mining.authorize\", \"params\": [\""
        + username + "\", \"" + password + "\"]}\n";
    if (!sendData(authorizeMsg)) {
        std::cerr << "send authorize failed" << std::endl;
        LOGI("send authorize failed");
        return false;
    }
    //std::cout << "Sent authorized: " << authorizeMsg;
    // Nhận phản hồi từ server
    /*std::string response = receiveData();
    std::cout << "Response: " << response << std::endl;*/
    return true;
}

void mining(work& workX, StratumClient& client) {
    stop_threads.store(false);
    mined.store(false);

    const int num_threads = 8;
    uint32_t total_nonce = 214000000;
    uint32_t range_per_thread = total_nonce / num_threads;

    LOGI("Count hashrate");
    unsigned long hashdone_count = 0;
    work* count_work = new work;
    *count_work = workX;
    int hashrate = scanhash_verus_count(0, count_work, 0, 4000000000, &hashdone_count)/2;
    LOGI("hashrate = %d", hashrate);
    char buf[128];
    snprintf(buf, sizeof(buf), "hashrate = %d", hashrate);
    add_status_line(std::string(buf));

//    range_per_thread = hashrate * 60; // cho tối đa 60

    LOGI("Start work");

    uint32_t start_range = deviceId.load() * total_nonce;

    snprintf(buf, sizeof(buf), "start_range = %u ; deviceId = %d", start_range, deviceId.load());
    add_status_line(buf);

    for (int i = 0; i < num_threads; i++) {
        uint32_t start_nonce = start_range + i * range_per_thread;
        uint32_t end_nonce = start_range + (i + 1) * range_per_thread;

        // clone work vì mỗi thread cần vùng nhớ riêng
        work* thread_work = new work;
        *thread_work = workX;

        workers.emplace_back([=, &client]() {

            unsigned long local_hashdone = 0;
            //thread_work->data[EQNONCE_OFFSET + 2] = start_nonce;
            scanhash_verus(i, thread_work, start_nonce, end_nonce, &local_hashdone);

            if (thread_work->valid_nonces > 0) {
                char* submitStr = NULL;
                submit_upstream_work(thread_work, &submitStr);

                //printf("submitStr = %s\n", submitStr);
                client.submitWork(std::string(submitStr));
                printf("\n Mined {%s}\n", thread_work->job_id);
                LOGI("Mined %s", thread_work->job_id);
                char buf[128];
                snprintf(buf, sizeof(buf), "Mined %s", thread_work->job_id);
                add_status_line(buf);
            };

            delete thread_work;
            });
    }

    // Join tất cả threads
    auto current_id = std::this_thread::get_id();
    for (auto& w : workers) {
        if (w.joinable() && w.get_id() != current_id) {
            w.join();
        }
    }

    workers.clear();
}

int connect_stratum(StratumClient& client, stratum_ctx *sctx, work *work)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return false;
    }
#endif

    //// Thông tin kết nối của Luckpool Stratum (cần kiểm tra lại thông số chính xác từ tài liệu của pool)
    //std::string host = "ap.luckpool.net";  // Địa chỉ server
    //int port = 3960; // Port kết nối (có thể thay đổi theo pool)

    //StratumClient client(host, port);

    if (!client.connectToServer()) {
        return 1;
    }

    // Thực hiện subscribe
    if (!client.subscribe()) {
        return 1;
    }

    // Thực hiện authorize
    // Thay "username.worker" và "password" bằng thông tin thực của bạn
    // RYSXNdVZkdFRvU3ySwdA3orQ1z1tEVpvRY
//    if (!client.authorize("RXxxZzRxYT68fXFPmMiR7xaQySv1hWtUNF.CPU", "x")) {
//        return 1;
//    }
    if (!client.authorize("RYSXNdVZkdFRvU3ySwdA3orQ1z1tEVpvRY.CPU", "x")) {
        return 1;
    }

    // Vòng lặp nhận dữ liệu liên tục từ server (có thể cần xử lý dữ liệu phức tạp hơn tùy mục đích)
    while (true) {
        std::string data = client.receiveData();

        if (!data.empty()) {

            std::stringstream ss(data);
            std::string line;
            while (std::getline(ss, line)) {
                json_error_t error;
                json_t* jsonObj;
                jsonObj = json_loads(line.c_str(), 0, &error);

                /*std::cout << "\n#line: \n" << line.c_str() << "\n#end-line\n" << std::endl;*/

                // Xử lý nonce1
                json_t* result = json_object_get(jsonObj, "result");
                if (result) {
                    LOGI("result: %s", line.c_str());
                    char buf[128];
                    snprintf(buf, sizeof(buf), "result: %s", line.c_str());
                    add_status_line(buf);
                    if (json_is_array(result)) {
                        stratum_parse_extranonce(sctx, result, 1);
                    }
                }

                // Xử lý job
                json_t* method = json_object_get(jsonObj, "method");
                if (method)  {
                    if (!strcmp(json_string_value(method), "mining.set_target")) {
                        std::cout << std::endl;
                        std::cout << "Action: " << json_string_value(method) << std::endl;

                        json_t* params = json_object_get(jsonObj, "params");
                        if (params) {
                            equi_stratum_set_target(sctx, params);
                        }
                    }
                    else if (!strcmp(json_string_value(method), "mining.notify")) {
                        std::cout << std::endl;
                        std::cout << "Action: " << json_string_value(method) << std::endl;

                        json_t* params = json_object_get(jsonObj, "params");

                        if (params) {
                            // Gen job mới
                            stratum_notify(sctx, params);

                            struct work work2 = *work;
                            int wcmplen = 76;
                            uint32_t* nonceptr = (uint32_t*)(((char*)work2.data) + wcmplen);
                            nonceptr = &work2.data[EQNONCE_OFFSET];
                            wcmplen = 4 + 32 + 32;

                            stratum_gen_work(sctx, &work2);

                            nonceptr[2] = rand() << 24 | rand() << 8 | 0;

                            // Thêm job vào job_queue
                            g_job_queue.addJob(work2);

                            // Chạy job mới
                            //mining(work2, client);
                        }
                    }
                }

                if (jsonObj) {
                    json_decref(jsonObj);
                }
            }
        }
    }

    return 0;
}