//
// Created by tuann on 4/8/2025.
//

#ifndef MYNEON_MY_LOG_H
#define MYNEON_MY_LOG_H

#include <android/log.h>
#define LOG_TAG "MyNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOG_HEX(label, data, len)                        \
    do {                                                 \
        char hexbuf[1024] = {0};                         \
        char *ptr = hexbuf;                              \
        for (int i = 0; i < (len); i++) {                \
            ptr += sprintf(ptr, "%02x", (data)[i]);      \
        }                                                \
        LOGI("%s: %s", (label), hexbuf);                 \
    } while (0)

#endif //MYNEON_MY_LOG_H
