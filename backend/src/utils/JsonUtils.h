#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <locale>
#include <codecvt>

namespace utils {

/**
 * @brief JSON工具类，处理UTF-8编码和解析
 */
class JsonUtils {
public:
    /**
     * @brief 安全解析JSON字符串，确保UTF-8编码正确处理
     * @param json_str JSON字符串
     * @return 解析结果，失败时抛出异常
     */
    static nlohmann::json parseJsonSafe(const std::string& json_str);

    /**
     * @brief 验证字符串是否为有效的UTF-8编码
     * @param str 待验证字符串
     * @return 是否为有效UTF-8
     */
    static bool isValidUtf8(const std::string& str);

    /**
     * @brief 清理并修复可能的UTF-8编码问题
     * @param str 原始字符串
     * @return 修复后的字符串
     */
    static std::string sanitizeUtf8(const std::string& str);

    /**
     * @brief 转换JSON到UTF-8字符串
     * @param json JSON对象
     * @return UTF-8编码的JSON字符串
     */
    static std::string toUtf8String(const nlohmann::json& json);

private:
    /**
     * @brief 检查字节是否为UTF-8序列的开始
     */
    static bool isUtf8Start(unsigned char byte);

    /**
     * @brief 检查字节是否为UTF-8序列的延续
     */
    static bool isUtf8Continuation(unsigned char byte);
};

} // namespace utils