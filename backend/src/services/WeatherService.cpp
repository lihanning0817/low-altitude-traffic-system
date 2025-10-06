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
    , baseUrl_("https://restapi.amap.com/v3") {

    std::cout << "[WeatherService] Initializing Weather Service (AMap API)" << std::endl;
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

    // Step 1: Get adcode from coordinates using reverse geocoding
    std::ostringstream geoUrl;
    geoUrl << baseUrl_ << "/geocode/regeo?key=" << apiKey_
           << "&location=" << lon << "," << lat;

    std::string geoResponse = makeHttpRequest(geoUrl.str());
    auto geoJson = nlohmann::json::parse(geoResponse);

    // Extract citycode or adcode for weather query
    std::string citycode = "110000"; // Default to Beijing
    if (geoJson.contains("regeocode") && geoJson["regeocode"].contains("addressComponent")) {
        auto addr = geoJson["regeocode"]["addressComponent"];
        if (addr.contains("citycode") && !addr["citycode"].get<std::string>().empty()) {
            // Convert citycode (like "010") to adcode format by looking up province
            if (addr.contains("adcode")) {
                std::string adcode = addr["adcode"].get<std::string>();
                // Use first 4 digits + "00" for city level adcode
                if (adcode.length() >= 4) {
                    citycode = adcode.substr(0, 4) + "00";
                }
            }
        }
    }

    // Step 2: Get weather using adcode
    std::ostringstream weatherUrl;
    weatherUrl << baseUrl_ << "/weather/weatherInfo?key=" << apiKey_
               << "&city=" << citycode << "&extensions=base";

    std::string weatherResponse = makeHttpRequest(weatherUrl.str());
    return parseWeatherResponse(weatherResponse);
}

nlohmann::json WeatherService::getForecastByCoords(double lat, double lon) {
    std::cout << "[WeatherService] Getting forecast for coords: "
              << lat << ", " << lon << std::endl;

    // Step 1: Get adcode from coordinates
    std::ostringstream geoUrl;
    geoUrl << baseUrl_ << "/geocode/regeo?key=" << apiKey_
           << "&location=" << lon << "," << lat;

    std::string geoResponse = makeHttpRequest(geoUrl.str());
    auto geoJson = nlohmann::json::parse(geoResponse);

    std::string citycode = "110000"; // Default to Beijing
    if (geoJson.contains("regeocode") && geoJson["regeocode"].contains("addressComponent")) {
        auto addr = geoJson["regeocode"]["addressComponent"];
        if (addr.contains("adcode")) {
            std::string adcode = addr["adcode"].get<std::string>();
            if (adcode.length() >= 4) {
                citycode = adcode.substr(0, 4) + "00";
            }
        }
    }

    // Step 2: Get weather forecast using adcode
    std::ostringstream weatherUrl;
    weatherUrl << baseUrl_ << "/weather/weatherInfo?key=" << apiKey_
               << "&city=" << citycode << "&extensions=all";

    std::string weatherResponse = makeHttpRequest(weatherUrl.str());
    return parseWeatherResponse(weatherResponse);
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
        // AMap API format (实时天气 extensions=base)
        if (weatherData.contains("lives") && weatherData["lives"].is_array() && !weatherData["lives"].empty()) {
            auto live = weatherData["lives"][0];

            // 位置信息
            formatted["location"] = live.value("city", "未知");

            // 天气状况
            formatted["condition"] = live.value("weather", "未知");

            // 温度信息 (高德返回字符串，需要转换)
            std::string temp_str = live.value("temperature", "0");
            formatted["temperature"] = std::stod(temp_str);
            formatted["feels_like"] = std::stod(temp_str); // 高德不提供体感温度，使用实际温度
            formatted["temp_min"] = std::stod(temp_str) - 2;
            formatted["temp_max"] = std::stod(temp_str) + 2;

            // 湿度
            std::string humidity_str = live.value("humidity", "0");
            formatted["humidity"] = std::stoi(humidity_str);

            // 气压 (高德不提供，使用标准值)
            formatted["pressure"] = 1013;

            // 风力信息
            std::string windpower = live.value("windpower", "0");
            // 转换风力等级到风速 (m/s), "≤3" -> 1.5, "4" -> 7.5, etc
            double wind_speed = 1.5; // default
            if (windpower.find("≤3") != std::string::npos) {
                wind_speed = 1.5;
            } else if (windpower.find("4") != std::string::npos) {
                wind_speed = 7.5;
            } else if (windpower.find("5") != std::string::npos) {
                wind_speed = 10.5;
            }
            formatted["wind_speed"] = wind_speed;

            // 风向 (高德返回中文如"东北"，需要转换为角度)
            std::string wind_dir = live.value("winddirection", "北");
            int wind_deg = 0; // 北
            if (wind_dir == "东北") wind_deg = 45;
            else if (wind_dir == "东") wind_deg = 90;
            else if (wind_dir == "东南") wind_deg = 135;
            else if (wind_dir == "南") wind_deg = 180;
            else if (wind_dir == "西南") wind_deg = 225;
            else if (wind_dir == "西") wind_deg = 270;
            else if (wind_dir == "西北") wind_deg = 315;
            formatted["wind_direction"] = wind_deg;

            // 能见度和云量 (高德不提供，使用估算值)
            formatted["visibility"] = 10000; // 默认10km
            formatted["cloudiness"] = 20; // 默认值

            // 图标映射 (根据天气状况映射到OpenWeatherMap图标)
            std::string condition = live.value("weather", "晴");
            std::string icon = "01d"; // 默认晴天
            if (condition.find("晴") != std::string::npos) icon = "01d";
            else if (condition.find("多云") != std::string::npos) icon = "02d";
            else if (condition.find("阴") != std::string::npos) icon = "03d";
            else if (condition.find("雨") != std::string::npos) icon = "10d";
            else if (condition.find("雪") != std::string::npos) icon = "13d";
            else if (condition.find("雾") != std::string::npos || condition.find("霾") != std::string::npos) icon = "50d";
            formatted["icon"] = icon;

            // 时间戳
            formatted["timestamp"] = std::time(nullptr);
        }
        // AMap API format (预报天气 extensions=all)
        else if (weatherData.contains("forecasts") && weatherData["forecasts"].is_array() && !weatherData["forecasts"].empty()) {
            auto forecast = weatherData["forecasts"][0];

            if (forecast.contains("casts") && forecast["casts"].is_array() && !forecast["casts"].empty()) {
                auto cast = forecast["casts"][0]; // 使用第一天的预报

                formatted["location"] = forecast.value("city", "未知");
                formatted["condition"] = cast.value("dayweather", "未知");

                std::string temp = cast.value("daytemp", "0");
                formatted["temperature"] = std::stod(temp);
                formatted["feels_like"] = std::stod(temp);
                formatted["temp_min"] = std::stod(cast.value("nighttemp", "0"));
                formatted["temp_max"] = std::stod(temp);
                formatted["humidity"] = 50; // 默认值
                formatted["pressure"] = 1013;

                // 风力转换
                std::string windpower = cast.value("daypower", "1-3");
                double wind_speed = 3.0;
                if (windpower.find("1-3") != std::string::npos) wind_speed = 3.0;
                else if (windpower.find("4-5") != std::string::npos) wind_speed = 9.0;
                else if (windpower.find("6-7") != std::string::npos) wind_speed = 15.0;
                formatted["wind_speed"] = wind_speed;
                formatted["wind_direction"] = 0;

                formatted["visibility"] = 10000;
                formatted["cloudiness"] = 20;

                // 图标映射
                std::string condition = cast.value("dayweather", "晴");
                std::string icon = "01d";
                if (condition.find("晴") != std::string::npos) icon = "01d";
                else if (condition.find("多云") != std::string::npos) icon = "02d";
                else if (condition.find("阴") != std::string::npos) icon = "03d";
                else if (condition.find("雨") != std::string::npos) icon = "10d";
                else if (condition.find("雪") != std::string::npos) icon = "13d";
                formatted["icon"] = icon;

                formatted["timestamp"] = std::time(nullptr);
            }
        }

        // 添加飞行安全评估（仅在有数据时）
        if (!formatted.empty()) {
            auto tempWeatherData = formatted; // 使用格式化后的数据进行安全评估
            tempWeatherData["wind"]["speed"] = formatted.value("wind_speed", 0.0);
            tempWeatherData["main"]["temp"] = formatted.value("temperature", 0.0);
            tempWeatherData["visibility"] = formatted.value("visibility", 10000);
            formatted["flight_safety"] = checkFlightSafety(tempWeatherData);
        }

    } catch (const std::exception& e) {
        std::cerr << "[WeatherService] Error formatting weather data: " << e.what() << std::endl;
        // 返回空对象而不是崩溃
        formatted = nlohmann::json::object();
    }

    return formatted;
}

} // namespace services
