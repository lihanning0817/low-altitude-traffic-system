#include "WeatherService.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace services {

// Callback函数用于接收HTTP响应数据
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

WeatherService::WeatherService(const std::string& apiKey)
    : apiKey_(apiKey)
    , baseUrl_("https://api.openweathermap.org/data/2.5") {

    std::cout << "[WeatherService] Initializing Weather Service" << std::endl;
}

std::string WeatherService::makeHttpRequest(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 开发环境可以关闭SSL验证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // 跟随重定向
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL request failed: " + error);
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (http_code != 200) {
        throw std::runtime_error("HTTP error: " + std::to_string(http_code));
    }

    return response;
}

nlohmann::json WeatherService::parseWeatherResponse(const std::string& response) {
    try {
        return nlohmann::json::parse(response);
    } catch (const std::exception& e) {
        std::cerr << "[WeatherService] Failed to parse weather response: " << e.what() << std::endl;
        throw std::runtime_error("Failed to parse weather API response");
    }
}

std::string WeatherService::buildApiUrl(
    const std::string& endpoint,
    const nlohmann::json& params
) {
    std::ostringstream url;
    url << baseUrl_ << "/" << endpoint << "?";

    for (auto it = params.begin(); it != params.end(); ++it) {
        if (it != params.begin()) {
            url << "&";
        }
        url << it.key() << "=" << it.value().get<std::string>();
    }

    url << "&appid=" << apiKey_;
    url << "&units=metric"; // 使用摄氏度
    url << "&lang=zh_cn";   // 中文描述

    return url.str();
}

nlohmann::json WeatherService::getCurrentWeatherByCity(
    const std::string& cityName,
    const std::string& countryCode
) {
    std::cout << "[WeatherService] Getting weather for city: " << cityName << std::endl;

    nlohmann::json params;
    if (!countryCode.empty()) {
        params["q"] = cityName + "," + countryCode;
    } else {
        params["q"] = cityName;
    }

    std::string url = buildApiUrl("weather", params);
    std::string response = makeHttpRequest(url);

    return parseWeatherResponse(response);
}

nlohmann::json WeatherService::getCurrentWeatherByCoords(double lat, double lon) {
    std::cout << "[WeatherService] Getting weather for coords: "
              << lat << ", " << lon << std::endl;

    nlohmann::json params;
    params["lat"] = std::to_string(lat);
    params["lon"] = std::to_string(lon);

    std::string url = buildApiUrl("weather", params);
    std::string response = makeHttpRequest(url);

    return parseWeatherResponse(response);
}

nlohmann::json WeatherService::getForecastByCoords(double lat, double lon) {
    std::cout << "[WeatherService] Getting forecast for coords: "
              << lat << ", " << lon << std::endl;

    nlohmann::json params;
    params["lat"] = std::to_string(lat);
    params["lon"] = std::to_string(lon);

    std::string url = buildApiUrl("forecast", params);
    std::string response = makeHttpRequest(url);

    return parseWeatherResponse(response);
}

nlohmann::json WeatherService::checkFlightSafety(const nlohmann::json& weatherData) {
    nlohmann::json result;
    result["safe"] = true;
    result["warnings"] = nlohmann::json::array();
    result["score"] = 100; // 安全评分 0-100

    try {
        // 检查主要天气状况
        if (weatherData.contains("weather") && weatherData["weather"].is_array() &&
            !weatherData["weather"].empty()) {
            auto weather = weatherData["weather"][0];
            std::string main = weather["main"].get<std::string>();

            // 恶劣天气条件
            if (main == "Thunderstorm") {
                result["safe"] = false;
                result["score"] = 0;
                result["warnings"].push_back("雷暴天气，禁止飞行");
            } else if (main == "Snow") {
                result["safe"] = false;
                result["score"] = 20;
                result["warnings"].push_back("降雪天气，不建议飞行");
            } else if (main == "Rain") {
                result["score"] = 50;
                result["warnings"].push_back("降雨天气，请谨慎飞行");
            } else if (main == "Fog" || main == "Mist") {
                result["score"] = 40;
                result["warnings"].push_back("能见度较低，请谨慎飞行");
            }
        }

        // 检查风速
        if (weatherData.contains("wind") && weatherData["wind"].contains("speed")) {
            double windSpeed = weatherData["wind"]["speed"].get<double>();

            if (windSpeed > 15) { // 风速大于15m/s
                result["safe"] = false;
                result["score"] = std::min(result["score"].get<int>(), 10);
                result["warnings"].push_back("风速过大（" + std::to_string((int)windSpeed) + " m/s），禁止飞行");
            } else if (windSpeed > 10) { // 风速10-15m/s
                result["score"] = std::min(result["score"].get<int>(), 50);
                result["warnings"].push_back("风速较大（" + std::to_string((int)windSpeed) + " m/s），请谨慎飞行");
            }
        }

        // 检查能见度
        if (weatherData.contains("visibility")) {
            int visibility = weatherData["visibility"].get<int>();

            if (visibility < 1000) { // 能见度小于1km
                result["safe"] = false;
                result["score"] = std::min(result["score"].get<int>(), 20);
                result["warnings"].push_back("能见度过低（" + std::to_string(visibility) + " m），禁止飞行");
            } else if (visibility < 5000) { // 能见度1-5km
                result["score"] = std::min(result["score"].get<int>(), 60);
                result["warnings"].push_back("能见度较低（" + std::to_string(visibility) + " m），请谨慎飞行");
            }
        }

        // 检查云量
        if (weatherData.contains("clouds") && weatherData["clouds"].contains("all")) {
            int cloudiness = weatherData["clouds"]["all"].get<int>();

            if (cloudiness > 80) { // 云量大于80%
                result["score"] = std::min(result["score"].get<int>(), 70);
                result["warnings"].push_back("云量较大（" + std::to_string(cloudiness) + "%），请注意观察");
            }
        }

        // 如果有警告但仍然安全，降低评分
        if (result["warnings"].size() > 0 && result["safe"].get<bool>()) {
            result["score"] = std::min(result["score"].get<int>(), 70);
        }

    } catch (const std::exception& e) {
        std::cerr << "[WeatherService] Error checking flight safety: " << e.what() << std::endl;
        result["safe"] = false;
        result["score"] = 0;
        result["warnings"].push_back("天气数据分析失败");
    }

    return result;
}

nlohmann::json WeatherService::formatWeatherData(const nlohmann::json& weatherData) {
    nlohmann::json formatted;

    try {
        // 基本信息
        if (weatherData.contains("name")) {
            formatted["location"] = weatherData["name"];
        }

        // 主要天气状况
        if (weatherData.contains("weather") && weatherData["weather"].is_array() &&
            !weatherData["weather"].empty()) {
            auto weather = weatherData["weather"][0];
            formatted["condition"] = weather["description"];
            formatted["icon"] = weather["icon"];
        }

        // 温度信息
        if (weatherData.contains("main")) {
            auto main = weatherData["main"];
            formatted["temperature"] = main["temp"];
            formatted["feels_like"] = main["feels_like"];
            formatted["temp_min"] = main["temp_min"];
            formatted["temp_max"] = main["temp_max"];
            formatted["humidity"] = main["humidity"];
            formatted["pressure"] = main["pressure"];
        }

        // 风力信息
        if (weatherData.contains("wind")) {
            auto wind = weatherData["wind"];
            formatted["wind_speed"] = wind["speed"];
            if (wind.contains("deg")) {
                formatted["wind_direction"] = wind["deg"];
            }
            if (wind.contains("gust")) {
                formatted["wind_gust"] = wind["gust"];
            }
        }

        // 能见度
        if (weatherData.contains("visibility")) {
            formatted["visibility"] = weatherData["visibility"];
        }

        // 云量
        if (weatherData.contains("clouds")) {
            formatted["cloudiness"] = weatherData["clouds"]["all"];
        }

        // 时间信息
        if (weatherData.contains("dt")) {
            formatted["timestamp"] = weatherData["dt"];
        }

        // 添加飞行安全评估
        formatted["flight_safety"] = checkFlightSafety(weatherData);

    } catch (const std::exception& e) {
        std::cerr << "[WeatherService] Error formatting weather data: " << e.what() << std::endl;
    }

    return formatted;
}

} // namespace services
