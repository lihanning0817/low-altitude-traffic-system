#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace config {

/**
 * @brief 配置管理类 - 单例模式
 * 负责加载和管理系统配置
 */
class Config {
private:
    static std::unique_ptr<Config> instance_;
    nlohmann::json config_data_;

    Config() = default;

public:
    // 禁用拷贝构造和赋值
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    /**
     * @brief 获取配置单例实例
     */
    static Config& getInstance();

    /**
     * @brief 从文件加载配置
     * @param config_file 配置文件路径
     * @return 是否加载成功
     */
    bool loadFromFile(const std::string& config_file);

    /**
     * @brief 从JSON字符串加载配置
     * @param json_str JSON配置字符串
     * @return 是否加载成功
     */
    bool loadFromString(const std::string& json_str);

    /**
     * @brief 获取字符串配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    std::string getString(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief 获取整数配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    int getInt(const std::string& key, int default_value = 0) const;

    /**
     * @brief 获取布尔配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    bool getBool(const std::string& key, bool default_value = false) const;

    /**
     * @brief 获取浮点数配置值
     * @param key 配置键
     * @param default_value 默认值
     * @return 配置值
     */
    double getDouble(const std::string& key, double default_value = 0.0) const;

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void set(const std::string& key, const nlohmann::json& value);

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return 是否存在
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief 获取所有配置
     * @return 配置JSON对象
     */
    const nlohmann::json& getAll() const;

    /**
     * @brief 重置为默认配置
     */
    void setDefaults();

private:
    /**
     * @brief 从嵌套键获取值（支持点分隔符）
     * @param key 键名（如 "database.host"）
     * @return JSON值的指针，不存在则返回nullptr
     */
    const nlohmann::json* getNestedValue(const std::string& key) const;
};

} // namespace config