#include "Config.h"
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

namespace config {

std::unique_ptr<Config> Config::instance_ = nullptr;

Config& Config::getInstance() {
    if (!instance_) {
        instance_ = std::unique_ptr<Config>(new Config());
        instance_->setDefaults();
    }
    return *instance_;
}

bool Config::loadFromFile(const std::string& config_file) {
    try {
        std::ifstream file(config_file);
        if (!file.is_open()) {
            spdlog::error("Failed to open config file: {}", config_file);
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        return loadFromString(buffer.str());
    }
    catch (const std::exception& e) {
        spdlog::error("Error loading config from file {}: {}", config_file, e.what());
        return false;
    }
}

bool Config::loadFromString(const std::string& json_str) {
    try {
        config_data_ = nlohmann::json::parse(json_str);
        spdlog::info("Configuration loaded successfully");
        return true;
    }
    catch (const nlohmann::json::parse_error& e) {
        spdlog::error("JSON parse error in config: {}", e.what());
        return false;
    }
    catch (const std::exception& e) {
        spdlog::error("Error loading config from string: {}", e.what());
        return false;
    }
}

const nlohmann::json* Config::getNestedValue(const std::string& key) const {
    std::istringstream iss(key);
    std::string token;
    const nlohmann::json* current = &config_data_;

    while (std::getline(iss, token, '.')) {
        if (current->contains(token)) {
            current = &(*current)[token];
        } else {
            return nullptr;
        }
    }

    return current;
}

std::string Config::getString(const std::string& key, const std::string& default_value) const {
    const auto* value = getNestedValue(key);
    if (value && value->is_string()) {
        return value->get<std::string>();
    }
    return default_value;
}

int Config::getInt(const std::string& key, int default_value) const {
    const auto* value = getNestedValue(key);
    if (value && value->is_number_integer()) {
        return value->get<int>();
    }
    return default_value;
}

bool Config::getBool(const std::string& key, bool default_value) const {
    const auto* value = getNestedValue(key);
    if (value && value->is_boolean()) {
        return value->get<bool>();
    }
    return default_value;
}

double Config::getDouble(const std::string& key, double default_value) const {
    const auto* value = getNestedValue(key);
    if (value && value->is_number()) {
        return value->get<double>();
    }
    return default_value;
}

void Config::set(const std::string& key, const nlohmann::json& value) {
    std::istringstream iss(key);
    std::string token;
    nlohmann::json* current = &config_data_;

    std::vector<std::string> keys;
    while (std::getline(iss, token, '.')) {
        keys.push_back(token);
    }

    for (size_t i = 0; i < keys.size() - 1; ++i) {
        if (!current->contains(keys[i])) {
            (*current)[keys[i]] = nlohmann::json::object();
        }
        current = &(*current)[keys[i]];
    }

    (*current)[keys.back()] = value;
}

bool Config::hasKey(const std::string& key) const {
    return getNestedValue(key) != nullptr;
}

const nlohmann::json& Config::getAll() const {
    return config_data_;
}

void Config::setDefaults() {
    config_data_ = nlohmann::json{
        {"server", {
            {"host", "0.0.0.0"},
            {"port", 8081},
            {"threads", 4},
            {"timeout", 30}
        }},
        {"database", {
            {"host", "localhost"},
            {"port", 33060},
            {"database", "low_altitude_traffic_system"},
            {"username", "root"},
            {"password", ""},
            {"connection_pool_size", 10},
            {"connection_timeout", 10}
        }},
        {"security", {
            {"jwt_secret", "your-jwt-secret-key-change-in-production"},
            {"jwt_expiration", 3600},
            {"password_min_length", 8}
        }},
        {"flight", {
            {"max_altitude", 300},
            {"min_altitude", 50},
            {"max_speed", 80},
            {"default_speed", 50}
        }},
        {"logging", {
            {"level", "info"},
            {"file", "logs/server.log"},
            {"max_size", "10MB"},
            {"max_files", 10}
        }},
        {"external_apis", {
            {"amap", {
                {"key", ""},
                {"base_url", "https://restapi.amap.com"},
                {"timeout", 5000}
            }},
            {"weather", {
                {"key", ""},
                {"base_url", "http://api.openweathermap.org"},
                {"timeout", 5000}
            }}
        }},
        {"cors", {
            {"enabled", true},
            {"origins", {"*"}},
            {"methods", {"GET", "POST", "PUT", "DELETE", "OPTIONS"}},
            {"headers", {"Content-Type", "Authorization"}}
        }}
    };

    spdlog::info("Default configuration set");
}

} // namespace config