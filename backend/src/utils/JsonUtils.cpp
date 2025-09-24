#include "JsonUtils.h"
#include <iostream>
#include <algorithm>
#include <sstream>

namespace utils {

nlohmann::json JsonUtils::parseJsonSafe(const std::string& json_str) {
    // 首先验证UTF-8编码
    if (!isValidUtf8(json_str)) {
        std::cerr << "[JsonUtils] Invalid UTF-8 detected, attempting to sanitize..." << std::endl;
        std::string sanitized = sanitizeUtf8(json_str);
        std::cerr << "[JsonUtils] Original: " << json_str << std::endl;
        std::cerr << "[JsonUtils] Sanitized: " << sanitized << std::endl;
        return nlohmann::json::parse(sanitized);
    }

    // 直接解析
    return nlohmann::json::parse(json_str);
}

bool JsonUtils::isValidUtf8(const std::string& str) {
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    size_t len = str.length();

    for (size_t i = 0; i < len; ++i) {
        unsigned char byte = bytes[i];

        // ASCII字符 (0xxxxxxx)
        if (byte <= 0x7F) {
            continue;
        }

        // 2字节UTF-8 (110xxxxx 10xxxxxx)
        if ((byte & 0xE0) == 0xC0) {
            if (i + 1 >= len || !isUtf8Continuation(bytes[i + 1])) {
                return false;
            }
            i += 1;
        }
        // 3字节UTF-8 (1110xxxx 10xxxxxx 10xxxxxx)
        else if ((byte & 0xF0) == 0xE0) {
            if (i + 2 >= len ||
                !isUtf8Continuation(bytes[i + 1]) ||
                !isUtf8Continuation(bytes[i + 2])) {
                return false;
            }
            i += 2;
        }
        // 4字节UTF-8 (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
        else if ((byte & 0xF8) == 0xF0) {
            if (i + 3 >= len ||
                !isUtf8Continuation(bytes[i + 1]) ||
                !isUtf8Continuation(bytes[i + 2]) ||
                !isUtf8Continuation(bytes[i + 3])) {
                return false;
            }
            i += 3;
        }
        // 无效的UTF-8起始字节
        else {
            return false;
        }
    }

    return true;
}

std::string JsonUtils::sanitizeUtf8(const std::string& str) {
    std::ostringstream result;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(str.c_str());
    size_t len = str.length();

    for (size_t i = 0; i < len; ++i) {
        unsigned char byte = bytes[i];

        // ASCII字符
        if (byte <= 0x7F) {
            result << static_cast<char>(byte);
            continue;
        }

        // 尝试解析UTF-8序列
        bool valid_sequence = false;

        // 2字节UTF-8
        if ((byte & 0xE0) == 0xC0 && i + 1 < len && isUtf8Continuation(bytes[i + 1])) {
            result << static_cast<char>(byte) << static_cast<char>(bytes[i + 1]);
            i += 1;
            valid_sequence = true;
        }
        // 3字节UTF-8
        else if ((byte & 0xF0) == 0xE0 && i + 2 < len &&
                 isUtf8Continuation(bytes[i + 1]) && isUtf8Continuation(bytes[i + 2])) {
            result << static_cast<char>(byte) << static_cast<char>(bytes[i + 1]) << static_cast<char>(bytes[i + 2]);
            i += 2;
            valid_sequence = true;
        }
        // 4字节UTF-8
        else if ((byte & 0xF8) == 0xF0 && i + 3 < len &&
                 isUtf8Continuation(bytes[i + 1]) && isUtf8Continuation(bytes[i + 2]) && isUtf8Continuation(bytes[i + 3])) {
            result << static_cast<char>(byte) << static_cast<char>(bytes[i + 1]) << static_cast<char>(bytes[i + 2]) << static_cast<char>(bytes[i + 3]);
            i += 3;
            valid_sequence = true;
        }

        // 如果不是有效的UTF-8序列，替换为占位符
        if (!valid_sequence) {
            std::cerr << "[JsonUtils] Invalid UTF-8 byte: 0x" << std::hex << static_cast<int>(byte) << std::dec << std::endl;
            result << "?";  // 替换无效字节
        }
    }

    return result.str();
}

std::string JsonUtils::toUtf8String(const nlohmann::json& json) {
    return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
}

bool JsonUtils::isUtf8Start(unsigned char byte) {
    return (byte & 0x80) == 0 ||           // ASCII
           (byte & 0xE0) == 0xC0 ||        // 2字节UTF-8开始
           (byte & 0xF0) == 0xE0 ||        // 3字节UTF-8开始
           (byte & 0xF8) == 0xF0;          // 4字节UTF-8开始
}

bool JsonUtils::isUtf8Continuation(unsigned char byte) {
    return (byte & 0xC0) == 0x80;          // 10xxxxxx
}

} // namespace utils