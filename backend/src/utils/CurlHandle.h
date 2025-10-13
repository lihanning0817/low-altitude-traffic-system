#pragma once

#include <curl/curl.h>
#include <stdexcept>
#include <string>

namespace utils {

/**
 * @brief RAII封装的CURL句柄管理类
 *
 * 自动管理CURL资源的生命周期，防止资源泄漏
 * 使用RAII(Resource Acquisition Is Initialization)模式
 */
class CurlHandle {
private:
    CURL* curl_;

public:
    /**
     * @brief 构造函数 - 初始化CURL句柄
     * @throws std::runtime_error 如果CURL初始化失败
     */
    CurlHandle() : curl_(curl_easy_init()) {
        if (!curl_) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    /**
     * @brief 析构函数 - 自动清理CURL资源
     */
    ~CurlHandle() {
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
    }

    /**
     * @brief 禁止拷贝构造
     */
    CurlHandle(const CurlHandle&) = delete;

    /**
     * @brief 禁止拷贝赋值
     */
    CurlHandle& operator=(const CurlHandle&) = delete;

    /**
     * @brief 移动构造函数
     */
    CurlHandle(CurlHandle&& other) noexcept : curl_(other.curl_) {
        other.curl_ = nullptr;
    }

    /**
     * @brief 移动赋值运算符
     */
    CurlHandle& operator=(CurlHandle&& other) noexcept {
        if (this != &other) {
            if (curl_) {
                curl_easy_cleanup(curl_);
            }
            curl_ = other.curl_;
            other.curl_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief 获取原始CURL指针
     * @return CURL* 原始指针，用于curl_easy_setopt等操作
     */
    CURL* get() {
        return curl_;
    }

    /**
     * @brief 检查CURL句柄是否有效
     * @return bool true表示有效，false表示无效
     */
    bool isValid() const {
        return curl_ != nullptr;
    }

    /**
     * @brief 执行HTTP请求
     * @return CURLcode 执行结果
     * @throws std::runtime_error 如果句柄无效
     */
    CURLcode perform() {
        if (!curl_) {
            throw std::runtime_error("Invalid CURL handle");
        }
        return curl_easy_perform(curl_);
    }

    /**
     * @brief 获取最后的错误信息
     * @param code CURLcode错误码
     * @return std::string 错误信息描述
     */
    static std::string getErrorString(CURLcode code) {
        return std::string(curl_easy_strerror(code));
    }
};

} // namespace utils
