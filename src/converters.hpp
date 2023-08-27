#include <Arduino.h>
#include <iostream>
#include <vector>
#include <cstdint>

int16_t fromByteToInt16(const char *source, size_t sourceLength, size_t start) {
    uint8_t *data;
    if (start + 1 > sourceLength) {
        // ソース配列の範囲を超えないようにチェック
        printf("Error: Source range exceeded.\n");
        return 0;
    }
    memcpy(data, source + start, 1);
    return (int16_t)(data[0]);
}

int16_t fromBytesToInt16(const char *source, size_t sourceLength, size_t start, size_t length) {
    uint8_t data[2];
    if (start + length > sourceLength) {
        // ソース配列の範囲を超えないようにチェック
        printf("Error: Source range exceeded.\n");
        return 0;
    }
    memcpy(data, source + start, length);
    return (int16_t)(data[0] | (data[1] << 8));
}

int32_t fromBytesToInt32(const char *source, size_t sourceLength, size_t start, size_t length) {
    uint8_t data[4];
    if (start + length > sourceLength) {
        // ソース配列の範囲を超えないようにチェック
        printf("Error: Source range exceeded.\n");
        return 0;
    }
    memcpy(data, source + start, length);
    return (int32_t)(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24));
}

int16_t fromBytesToInt16Reverse(const char *source, size_t sourceLength, size_t start, size_t length) {
    uint8_t data[2];
    if (start + length > sourceLength) {
        // ソース配列の範囲を超えないようにチェック
        printf("Error: Source range exceeded.\n");
        return 0;
    }
    memcpy(data, source + start, length);
    return (int16_t)(data[1] | (data[0] << 8));
}

int32_t fromBytesToInt32Reverse(const char *source, size_t sourceLength, size_t start, size_t length) {
    uint8_t data[4];
    if (start + length > sourceLength) {
        // ソース配列の範囲を超えないようにチェック
        printf("Error: Source range exceeded.\n");
        return 0;
    }
    memcpy(data, source + start, length);
    return (int32_t)(data[3] | (data[2] << 8) | (data[1] << 16) | (data[0] << 24));
}