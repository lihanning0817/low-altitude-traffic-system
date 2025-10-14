#include "WeatherService.h"
#include "utils/CurlHandle.h"
#include "utils/ParamParser.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <map>

namespace services {

// CURL配置常量
namespace {
    constexpr long HTTP_TIMEOUT = 30L;              // HTTP请求超时时间（秒）
    constexpr long HTTP_CONNECT_TIMEOUT = 10L;     // HTTP连接超时时间（秒）
    constexpr long SSL_VERIFY_PEER = 1L;           // 启用SSL证书验证
    constexpr long SSL_VERIFY_HOST = 2L;           // 启用SSL主机名验证
    constexpr long FOLLOW_REDIRECT = 1L;           // 跟随HTTP重定向
    constexpr long VERBOSE_OFF = 0L;               // 关闭详细输出
    constexpr long HTTP_OK = 200;                  // HTTP成功状态码
}

// 天气安全评分常量
namespace {
    constexpr int MAX_SAFETY_SCORE = 100;          // 最高安全评分
    constexpr int THUNDERSTORM_SCORE = 0;          // 雷暴天气评分
    constexpr int SNOW_SCORE = 20;                 // 降雪天气评分
    constexpr int HEAVY_RAIN_SCORE = 30;           // 强降雨天气评分
    constexpr int LIGHT_RAIN_SCORE = 50;           // 小雨天气评分
    constexpr int FOG_SCORE = 40;                  // 雾霾天气评分
    constexpr int MODERATE_WIND_SCORE = 70;        // 中等风速评分上限
    constexpr int HIGH_WIND_SCORE = 50;            // 大风评分上限
    constexpr int EXTREME_WIND_SCORE = 10;         // 极大风评分上限
    constexpr int LOW_VISIBILITY_SCORE = 20;       // 低能见度评分上限
    constexpr int MEDIUM_VISIBILITY_SCORE = 60;    // 中等能见度评分上限
    constexpr int WARNING_MAX_SCORE = 70;          // 有警告时的最高评分
}

// 天气阈值常量
namespace {
    constexpr double WIND_SPEED_MODERATE = 7.0;    // 中等风速阈值（m/s）
    constexpr double WIND_SPEED_HIGH = 10.0;       // 大风阈值（m/s）
    constexpr double WIND_SPEED_EXTREME = 15.0;    // 极大风阈值（m/s）
    constexpr int VISIBILITY_LOW = 1000;           // 低能见度阈值（米）
    constexpr int VISIBILITY_MEDIUM = 5000;        // 中等能见度阈值（米）
    constexpr int VISIBILITY_DEFAULT = 10000;      // 默认能见度（米）
}

// 默认天气参数常量
namespace {
    constexpr int DEFAULT_PRESSURE = 1013;         // 默认气压（hPa）
    constexpr int DEFAULT_CLOUDINESS = 20;         // 默认云量（%）
    constexpr int DEFAULT_HUMIDITY = 50;           // 默认湿度（%）
    constexpr double DEFAULT_WIND_SPEED = 3.0;     // 默认风速（m/s）
    constexpr int DEFAULT_WIND_DIRECTION = 0;      // 默认风向（北）
}

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
    std::string response;

    // 使用RAII包装的CurlHandle，自动管理CURL资源
    utils::CurlHandle curl;

    if (!curl.isValid()) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    // 配置CURL选项
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, HTTP_TIMEOUT);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, HTTP_CONNECT_TIMEOUT);

    // 启用SSL验证以提高安全性
    // 验证服务器证书的真实性（防止中间人攻击）
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, SSL_VERIFY_PEER);
    // 验证服务器证书中的主机名（防止DNS欺骗）
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, SSL_VERIFY_HOST);

    // 如果系统CA证书路径不标准，可以手动指定证书路径
    // curl_easy_setopt(curl.get(), CURLOPT_CAINFO, "/path/to/ca-bundle.crt");

    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, FOLLOW_REDIRECT); // 跟随重定向
    curl_easy_setopt(curl.get(), CURLOPT_VERBOSE, VERBOSE_OFF);

    // 执行HTTP请求
    CURLcode res = curl.perform();

    if (res != CURLE_OK) {
        // CurlHandle析构时会自动清理资源，无需手动cleanup
        throw std::runtime_error("CURL request failed: " + utils::CurlHandle::getErrorString(res));
    }

    // 检查HTTP响应码
    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    // CurlHandle析构时会自动清理资源

    if (http_code != HTTP_OK) {
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

    // 高德地图API使用key参数而不是appid
    url << "key=" << apiKey_;

    for (auto it = params.begin(); it != params.end(); ++it) {
        url << "&";
        url << it.key() << "=" << it.value().get<std::string>();
    }

    // 注意：高德地图API不支持units和lang参数，已移除

    return url.str();
}

nlohmann::json WeatherService::getCurrentWeatherByCity(
    const std::string& cityName,
    const std::string& countryCode
) {
    std::cout << "[WeatherService] Getting weather for city: " << cityName << std::endl;

    // 城市名称到城市代码(adcode)的映射
    // 高德地图API要求使用城市代码而非城市名称
    static const std::map<std::string, std::string> cityCodes = {
        {"北京", "110000"}, {"上海", "310000"}, {"天津", "120000"}, {"重庆", "500000"},
        {"沈阳", "210100"}, {"大连", "210200"}, {"长春", "220100"}, {"哈尔滨", "230100"},
        {"南京", "320100"}, {"杭州", "330100"}, {"宁波", "330200"}, {"苏州", "320500"},
        {"武汉", "420100"}, {"长沙", "430100"}, {"广州", "440100"}, {"深圳", "440300"},
        {"成都", "510100"}, {"西安", "610100"}, {"郑州", "410100"}, {"济南", "370100"},
        {"青岛", "370200"}, {"厦门", "350200"}, {"福州", "350100"}, {"昆明", "530100"},
        {"贵阳", "520100"}, {"南宁", "450100"}, {"太原", "140100"}, {"石家庄", "130100"},
        {"兰州", "620100"}, {"西宁", "630100"}, {"银川", "640100"}, {"乌鲁木齐", "650100"},
        {"呼和浩特", "150100"}, {"拉萨", "540100"}, {"海口", "460100"}, {"三亚", "460200"},
        {"合肥", "340100"}, {"南昌", "360100"}
    };

    // 尝试从映射表中获取城市代码
    std::string cityCode = "110000"; // 默认北京
    auto it = cityCodes.find(cityName);
    if (it != cityCodes.end()) {
        cityCode = it->second;
    }

    // 使用高德地图天气API的正确endpoint和参数
    std::ostringstream weatherUrl;
    weatherUrl << baseUrl_ << "/weather/weatherInfo?key=" << apiKey_
               << "&city=" << cityCode << "&extensions=base";

    std::cout << "[WeatherService] Requesting weather URL: " << weatherUrl.str() << std::endl;

    std::string weatherResponse = makeHttpRequest(weatherUrl.str());
    return parseWeatherResponse(weatherResponse);
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
    result["score"] = MAX_SAFETY_SCORE; // 安全评分 0-100

    try {
        // 支持两种格式：高德格式（lives数组）和已格式化的数据
        std::string condition;
        double windSpeed = 0.0;
        int visibility = VISIBILITY_DEFAULT;

        // 检查是否是高德API格式（lives数组）
        if (weatherData.contains("lives") && weatherData["lives"].is_array() && !weatherData["lives"].empty()) {
            auto live = weatherData["lives"][0];
            condition = live.value("weather", "");

            // 解析风力等级
            std::string windpower = live.value("windpower", "0");
            if (windpower.find("≤3") != std::string::npos) {
                windSpeed = 1.5;
            } else if (windpower.find("4") != std::string::npos) {
                windSpeed = 7.5;
            } else if (windpower.find("5") != std::string::npos) {
                windSpeed = 10.5;
            } else if (windpower.find("6") != std::string::npos) {
                windSpeed = 13.5;
            } else if (windpower.find("7") != std::string::npos || windpower.find("8") != std::string::npos) {
                windSpeed = 17.0;
            }
        }
        // 检查是否是已格式化的数据
        else if (weatherData.contains("condition")) {
            condition = weatherData.value("condition", "");
            windSpeed = weatherData.value("wind_speed", 0.0);
            visibility = weatherData.value("visibility", VISIBILITY_DEFAULT);
        }

        // 检查恶劣天气条件（中文）
        if (condition.find("雷") != std::string::npos || condition.find("暴") != std::string::npos) {
            result["safe"] = false;
            result["score"] = THUNDERSTORM_SCORE;
            result["warnings"].push_back("雷暴天气，禁止飞行");
        } else if (condition.find("雪") != std::string::npos) {
            result["safe"] = false;
            result["score"] = SNOW_SCORE;
            result["warnings"].push_back("降雪天气，不建议飞行");
        } else if (condition.find("雨") != std::string::npos) {
            if (condition.find("大雨") != std::string::npos || condition.find("暴雨") != std::string::npos) {
                result["safe"] = false;
                result["score"] = HEAVY_RAIN_SCORE;
                result["warnings"].push_back("强降雨天气，禁止飞行");
            } else {
                result["score"] = LIGHT_RAIN_SCORE;
                result["warnings"].push_back("降雨天气，请谨慎飞行");
            }
        } else if (condition.find("雾") != std::string::npos || condition.find("霾") != std::string::npos) {
            result["score"] = FOG_SCORE;
            result["warnings"].push_back("能见度较低，请谨慎飞行");
        }

        // 检查风速
        if (windSpeed > WIND_SPEED_EXTREME) { // 风速大于15m/s
            result["safe"] = false;
            result["score"] = std::min(result["score"].get<int>(), EXTREME_WIND_SCORE);
            result["warnings"].push_back("风速过大（" + std::to_string((int)windSpeed) + " m/s），禁止飞行");
        } else if (windSpeed > WIND_SPEED_HIGH) { // 风速10-15m/s
            result["score"] = std::min(result["score"].get<int>(), HIGH_WIND_SCORE);
            result["warnings"].push_back("风速较大（" + std::to_string((int)windSpeed) + " m/s），请谨慎飞行");
        } else if (windSpeed > WIND_SPEED_MODERATE) { // 风速7-10m/s
            result["score"] = std::min(result["score"].get<int>(), MODERATE_WIND_SCORE);
            result["warnings"].push_back("风速较大，请注意飞行安全");
        }

        // 检查能见度
        if (visibility < VISIBILITY_LOW) { // 能见度小于1km
            result["safe"] = false;
            result["score"] = std::min(result["score"].get<int>(), LOW_VISIBILITY_SCORE);
            result["warnings"].push_back("能见度过低（" + std::to_string(visibility) + " m），禁止飞行");
        } else if (visibility < VISIBILITY_MEDIUM) { // 能见度1-5km
            result["score"] = std::min(result["score"].get<int>(), MEDIUM_VISIBILITY_SCORE);
            result["warnings"].push_back("能见度较低（" + std::to_string(visibility / 1000) + " km），请谨慎飞行");
        }

        // 如果有警告但仍然安全，确保评分不会太高
        if (result["warnings"].size() > 0 && result["safe"].get<bool>()) {
            result["score"] = std::min(result["score"].get<int>(), WARNING_MAX_SCORE);
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

            // 温度信息 (高德返回字符串，需要安全转换)
            std::string temp_str = live.value("temperature", "0");
            double temp = utils::ParamParser::parseDouble(temp_str, 0.0, -50.0, 60.0);
            formatted["temperature"] = temp;
            formatted["feels_like"] = temp; // 高德不提供体感温度，使用实际温度
            formatted["temp_min"] = temp - 2;
            formatted["temp_max"] = temp + 2;

            // 湿度
            std::string humidity_str = live.value("humidity", "0");
            formatted["humidity"] = utils::ParamParser::parseInt(humidity_str, 0, 0, 100);

            // 气压 (高德不提供，使用标准值)
            formatted["pressure"] = DEFAULT_PRESSURE;

            // 风力信息
            std::string windpower = live.value("windpower", "0");
            // 转换风力等级到风速 (m/s), "≤3" -> 1.5, "4" -> 7.5, etc
            double wind_speed = 1.5; // 微风级别默认值
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
            int wind_deg = DEFAULT_WIND_DIRECTION; // 北
            if (wind_dir == "东北") wind_deg = 45;
            else if (wind_dir == "东") wind_deg = 90;
            else if (wind_dir == "东南") wind_deg = 135;
            else if (wind_dir == "南") wind_deg = 180;
            else if (wind_dir == "西南") wind_deg = 225;
            else if (wind_dir == "西") wind_deg = 270;
            else if (wind_dir == "西北") wind_deg = 315;
            formatted["wind_direction"] = wind_deg;

            // 能见度和云量 (高德不提供，使用估算值)
            formatted["visibility"] = VISIBILITY_DEFAULT; // 默认10km
            formatted["cloudiness"] = DEFAULT_CLOUDINESS; // 默认值

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

                // 安全地转换温度字符串
                std::string temp = cast.value("daytemp", "0");
                std::string nighttemp = cast.value("nighttemp", "0");
                double day_temp = utils::ParamParser::parseDouble(temp, 0.0, -50.0, 60.0);
                double night_temp = utils::ParamParser::parseDouble(nighttemp, 0.0, -50.0, 60.0);
                formatted["temperature"] = day_temp;
                formatted["feels_like"] = day_temp;
                formatted["temp_min"] = night_temp;
                formatted["temp_max"] = day_temp;

                formatted["humidity"] = DEFAULT_HUMIDITY; // 默认值
                formatted["pressure"] = DEFAULT_PRESSURE;

                // 风力转换
                std::string windpower = cast.value("daypower", "1-3");
                double wind_speed = DEFAULT_WIND_SPEED;
                if (windpower.find("1-3") != std::string::npos) wind_speed = DEFAULT_WIND_SPEED;
                else if (windpower.find("4-5") != std::string::npos) wind_speed = 9.0;
                else if (windpower.find("6-7") != std::string::npos) wind_speed = 15.0;
                else if (windpower.find("8-9") != std::string::npos) wind_speed = 20.0;
                formatted["wind_speed"] = wind_speed;
                formatted["wind_direction"] = DEFAULT_WIND_DIRECTION;

                formatted["visibility"] = VISIBILITY_DEFAULT;
                formatted["cloudiness"] = DEFAULT_CLOUDINESS;

                // 图标映射
                std::string condition = cast.value("dayweather", "晴");
                std::string icon = "01d";
                if (condition.find("晴") != std::string::npos) icon = "01d";
                else if (condition.find("多云") != std::string::npos) icon = "02d";
                else if (condition.find("阴") != std::string::npos) icon = "03d";
                else if (condition.find("雨") != std::string::npos) icon = "10d";
                else if (condition.find("雪") != std::string::npos) icon = "13d";
                else if (condition.find("雾") != std::string::npos || condition.find("霾") != std::string::npos) icon = "50d";
                formatted["icon"] = icon;

                formatted["timestamp"] = std::time(nullptr);
            }
        }

        // 添加飞行安全评估（仅在有数据时）
        // 直接使用格式化后的数据，checkFlightSafety已经支持这种格式
        if (!formatted.empty()) {
            formatted["flight_safety"] = checkFlightSafety(formatted);
        }

    } catch (const std::exception& e) {
        std::cerr << "[WeatherService] Error formatting weather data: " << e.what() << std::endl;
        // 返回空对象而不是崩溃
        formatted = nlohmann::json::object();
    }

    return formatted;
}

} // namespace services
