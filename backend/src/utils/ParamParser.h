#pragma once

#include <string>
#include <optional>
#include <algorithm>
#include <cctype>
#include <cstdint>

namespace utils {

/**
 * @brief 安全的参数解析工具类
 *
 * 提供安全的字符串到数值类型转换，避免std::stoi等函数的异常问题
 * 所有解析函数都返回std::optional，失败时返回std::nullopt
 */
class ParamParser {
public:
    /**
     * @brief 安全解析整数
     * @param str 输入字符串
     * @param defaultValue 默认值（解析失败时返回）
     * @param minValue 最小值限制（可选）
     * @param maxValue 最大值限制（可选）
     * @return int 解析结果或默认值
     */
    static int parseInt(const std::string& str,
                       int defaultValue = 0,
                       std::optional<int> minValue = std::nullopt,
                       std::optional<int> maxValue = std::nullopt) {
        try {
            // 去除前后空格
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return defaultValue;
            }

            // 检查是否为有效的整数格式
            if (!isValidInteger(trimmed)) {
                return defaultValue;
            }

            // 尝试转换
            size_t pos = 0;
            int value = std::stoi(trimmed, &pos);

            // 确保整个字符串都被解析
            if (pos != trimmed.length()) {
                return defaultValue;
            }

            // 应用范围限制
            if (minValue.has_value() && value < minValue.value()) {
                value = minValue.value();
            }
            if (maxValue.has_value() && value > maxValue.value()) {
                value = maxValue.value();
            }

            return value;
        } catch (...) {
            // 捕获所有异常，返回默认值
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析长整数
     * @param str 输入字符串
     * @param defaultValue 默认值（解析失败时返回）
     * @param minValue 最小值限制（可选）
     * @param maxValue 最大值限制（可选）
     * @return long 解析结果或默认值
     */
    static long parseLong(const std::string& str,
                         long defaultValue = 0L,
                         std::optional<long> minValue = std::nullopt,
                         std::optional<long> maxValue = std::nullopt) {
        try {
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return defaultValue;
            }

            if (!isValidInteger(trimmed)) {
                return defaultValue;
            }

            size_t pos = 0;
            long value = std::stol(trimmed, &pos);

            if (pos != trimmed.length()) {
                return defaultValue;
            }

            if (minValue.has_value() && value < minValue.value()) {
                value = minValue.value();
            }
            if (maxValue.has_value() && value > maxValue.value()) {
                value = maxValue.value();
            }

            return value;
        } catch (...) {
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析长长整数（int64_t）
     * @param str 输入字符串
     * @param defaultValue 默认值（解析失败时返回）
     * @param minValue 最小值限制（可选）
     * @param maxValue 最大值限制（可选）
     * @return int64_t 解析结果或默认值
     */
    static int64_t parseLongLong(const std::string& str,
                                  int64_t defaultValue = 0LL,
                                  std::optional<int64_t> minValue = std::nullopt,
                                  std::optional<int64_t> maxValue = std::nullopt) {
        try {
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return defaultValue;
            }

            if (!isValidInteger(trimmed)) {
                return defaultValue;
            }

            size_t pos = 0;
            int64_t value = std::stoll(trimmed, &pos);

            if (pos != trimmed.length()) {
                return defaultValue;
            }

            if (minValue.has_value() && value < minValue.value()) {
                value = minValue.value();
            }
            if (maxValue.has_value() && value > maxValue.value()) {
                value = maxValue.value();
            }

            return value;
        } catch (const std::invalid_argument&) {
            return defaultValue;
        } catch (const std::out_of_range&) {
            return defaultValue;
        } catch (...) {
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析浮点数
     * @param str 输入字符串
     * @param defaultValue 默认值（解析失败时返回）
     * @param minValue 最小值限制（可选）
     * @param maxValue 最大值限制（可选）
     * @return double 解析结果或默认值
     */
    static double parseDouble(const std::string& str,
                             double defaultValue = 0.0,
                             std::optional<double> minValue = std::nullopt,
                             std::optional<double> maxValue = std::nullopt) {
        try {
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return defaultValue;
            }

            if (!isValidDouble(trimmed)) {
                return defaultValue;
            }

            size_t pos = 0;
            double value = std::stod(trimmed, &pos);

            if (pos != trimmed.length()) {
                return defaultValue;
            }

            if (minValue.has_value() && value < minValue.value()) {
                value = minValue.value();
            }
            if (maxValue.has_value() && value > maxValue.value()) {
                value = maxValue.value();
            }

            return value;
        } catch (...) {
            return defaultValue;
        }
    }

    /**
     * @brief 安全解析布尔值
     * @param str 输入字符串（支持: "true", "false", "1", "0", "yes", "no"）
     * @param defaultValue 默认值（解析失败时返回）
     * @return bool 解析结果或默认值
     */
    static bool parseBool(const std::string& str, bool defaultValue = false) {
        std::string lower = toLower(trim(str));

        if (lower == "true" || lower == "1" || lower == "yes") {
            return true;
        }
        if (lower == "false" || lower == "0" || lower == "no") {
            return false;
        }

        return defaultValue;
    }

    /**
     * @brief 安全解析可选整数
     * @param str 输入字符串
     * @return std::optional<int> 成功返回值，失败返回std::nullopt
     */
    static std::optional<int> parseOptionalInt(const std::string& str) {
        try {
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return std::nullopt;
            }

            if (!isValidInteger(trimmed)) {
                return std::nullopt;
            }

            size_t pos = 0;
            int value = std::stoi(trimmed, &pos);

            if (pos != trimmed.length()) {
                return std::nullopt;
            }

            return value;
        } catch (...) {
            return std::nullopt;
        }
    }

    /**
     * @brief 安全解析可选浮点数
     * @param str 输入字符串
     * @return std::optional<double> 成功返回值，失败返回std::nullopt
     */
    static std::optional<double> parseOptionalDouble(const std::string& str) {
        try {
            std::string trimmed = trim(str);
            if (trimmed.empty()) {
                return std::nullopt;
            }

            if (!isValidDouble(trimmed)) {
                return std::nullopt;
            }

            size_t pos = 0;
            double value = std::stod(trimmed, &pos);

            if (pos != trimmed.length()) {
                return std::nullopt;
            }

            return value;
        } catch (...) {
            return std::nullopt;
        }
    }

private:
    /**
     * @brief 去除字符串前后空格
     */
    static std::string trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(),
                                      [](unsigned char ch) { return std::isspace(ch); });
        auto end = std::find_if_not(str.rbegin(), str.rend(),
                                    [](unsigned char ch) { return std::isspace(ch); }).base();
        return (start < end) ? std::string(start, end) : std::string();
    }

    /**
     * @brief 转换为小写
     */
    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /**
     * @brief 检查是否为有效的整数格式
     */
    static bool isValidInteger(const std::string& str) {
        if (str.empty()) {
            return false;
        }

        size_t i = 0;
        // 允许符号
        if (str[0] == '+' || str[0] == '-') {
            i = 1;
        }

        // 至少要有一个数字
        if (i >= str.length()) {
            return false;
        }

        // 检查剩余字符都是数字
        for (; i < str.length(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(str[i]))) {
                return false;
            }
        }

        return true;
    }

    /**
     * @brief 检查是否为有效的浮点数格式
     */
    static bool isValidDouble(const std::string& str) {
        if (str.empty()) {
            return false;
        }

        size_t i = 0;
        bool hasDigit = false;
        bool hasDecimal = false;
        bool hasExponent = false;

        // 允许符号
        if (str[0] == '+' || str[0] == '-') {
            i = 1;
        }

        for (; i < str.length(); ++i) {
            char c = str[i];

            if (std::isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;
            } else if (c == '.') {
                if (hasDecimal || hasExponent) {
                    return false; // 只能有一个小数点，且不能在指数后
                }
                hasDecimal = true;
            } else if (c == 'e' || c == 'E') {
                if (hasExponent || !hasDigit) {
                    return false; // 只能有一个指数符号，且前面必须有数字
                }
                hasExponent = true;
                hasDigit = false; // 指数后必须有数字
                // 指数后可以有符号
                if (i + 1 < str.length() && (str[i + 1] == '+' || str[i + 1] == '-')) {
                    i++;
                }
            } else {
                return false; // 无效字符
            }
        }

        return hasDigit;
    }
};

} // namespace utils
