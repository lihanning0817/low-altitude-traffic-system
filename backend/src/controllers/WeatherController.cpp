#include "WeatherController.h"
#include "utils/HttpResponse.h"
#include <iostream>
#include <sstream>
#include <regex>

namespace controllers {

WeatherController::WeatherController(
    std::shared_ptr<services::WeatherService> weatherService,
    std::shared_ptr<auth::JwtService> jwtService
) : weatherService_(weatherService), jwtService_(jwtService) {
    std::cout << "[WeatherController] Initializing Weather Controller" << std::endl;
}

std::string WeatherController::extractBearerToken(const http::request<http::string_body>& req) {
    auto it = req.find(http::field::authorization);
    if (it != req.end()) {
        std::string auth_header = it->value();
        const std::string bearer_prefix = "Bearer ";
        if (auth_header.substr(0, bearer_prefix.length()) == bearer_prefix) {
            return auth_header.substr(bearer_prefix.length());
        }
    }
    return "";
}

std::optional<int64_t> WeatherController::validateTokenAndGetUserId(
    const http::request<http::string_body>& req
) {
    std::string token = extractBearerToken(req);
    if (token.empty()) {
        return std::nullopt;
    }

    try {
        auto validationResult = jwtService_->validateToken(token);
        if (validationResult == auth::JwtValidationResult::VALID) {
            auto tokenInfo = jwtService_->parseToken(token);
            if (tokenInfo.has_value()) {
                return tokenInfo->user_id;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[WeatherController] JWT validation error: " << e.what() << std::endl;
    }

    return std::nullopt;
}

std::map<std::string, std::string> WeatherController::parseQueryParams(
    const http::request<http::string_body>& req
) {
    std::map<std::string, std::string> params;
    std::string target = req.target();
    size_t query_pos = target.find('?');

    if (query_pos != std::string::npos) {
        std::string query = target.substr(query_pos + 1);
        std::stringstream ss(query);
        std::string param;

        while (std::getline(ss, param, '&')) {
            size_t eq_pos = param.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = param.substr(0, eq_pos);
                std::string value = param.substr(eq_pos + 1);
                params[key] = value;
            }
        }
    }

    return params;
}

http::response<http::string_body> WeatherController::getCurrentWeather(
    const http::request<http::string_body>& req
) {
    std::cout << "[WeatherController] GET /api/v1/weather/current" << std::endl;

    try {
        // 验证JWT Token（可选，根据需求决定是否需要认证）
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录访问天气信息");
        }

        // 解析查询参数
        auto params = parseQueryParams(req);

        if (params.find("city") == params.end()) {
            return utils::HttpResponse::createErrorResponse("缺少城市参数");
        }

        std::string city = params["city"];
        std::string country = params.count("country") ? params["country"] : "";

        // 获取天气数据
        auto weatherData = weatherService_->getCurrentWeatherByCity(city, country);
        auto formattedData = weatherService_->formatWeatherData(weatherData);

        nlohmann::json response_data;
        response_data["weather"] = formattedData;

        return utils::HttpResponse::createSuccessResponse(response_data, "获取天气信息成功");

    } catch (const std::exception& e) {
        std::cerr << "[WeatherController] Error getting weather: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取天气信息失败");
    }
}

http::response<http::string_body> WeatherController::getCurrentWeatherByCoords(
    const http::request<http::string_body>& req
) {
    std::cout << "[WeatherController] GET /api/v1/weather/current/coords" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录访问天气信息");
        }

        // 解析查询参数
        auto params = parseQueryParams(req);

        if (params.find("lat") == params.end() || params.find("lon") == params.end()) {
            return utils::HttpResponse::createErrorResponse("缺少坐标参数");
        }

        double lat = std::stod(params["lat"]);
        double lon = std::stod(params["lon"]);

        // 获取天气数据
        auto weatherData = weatherService_->getCurrentWeatherByCoords(lat, lon);
        std::cout << "[WeatherController] Raw weather data: " << weatherData.dump() << std::endl;

        auto formattedData = weatherService_->formatWeatherData(weatherData);
        std::cout << "[WeatherController] Formatted weather data: " << formattedData.dump() << std::endl;

        nlohmann::json response_data;
        response_data["weather"] = formattedData;

        return utils::HttpResponse::createSuccessResponse(response_data, "获取天气信息成功");

    } catch (const std::exception& e) {
        std::cerr << "[WeatherController] Error getting weather by coords: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取天气信息失败");
    }
}

http::response<http::string_body> WeatherController::getForecast(
    const http::request<http::string_body>& req
) {
    std::cout << "[WeatherController] GET /api/v1/weather/forecast" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录访问天气预报");
        }

        // 解析查询参数
        auto params = parseQueryParams(req);

        if (params.find("lat") == params.end() || params.find("lon") == params.end()) {
            return utils::HttpResponse::createErrorResponse("缺少坐标参数");
        }

        double lat = std::stod(params["lat"]);
        double lon = std::stod(params["lon"]);

        // 获取天气预报数据
        auto forecastData = weatherService_->getForecastByCoords(lat, lon);

        // 格式化预报数据
        nlohmann::json formattedForecast = nlohmann::json::array();

        // AMap API format (forecasts[].casts[])
        if (forecastData.contains("forecasts") && forecastData["forecasts"].is_array() &&
            !forecastData["forecasts"].empty()) {
            auto forecast = forecastData["forecasts"][0];
            if (forecast.contains("casts") && forecast["casts"].is_array()) {
                for (const auto& cast : forecast["casts"]) {
                    nlohmann::json item;
                    item["dayweather"] = cast.value("dayweather", "未知");
                    item["daytemp"] = cast.value("daytemp", "0");
                    item["nighttemp"] = cast.value("nighttemp", "0");
                    item["daypower"] = cast.value("daypower", "1-3");
                    item["date"] = cast.value("date", "");

                    // Format as single item for compatibility
                    nlohmann::json formatted;
                    formatted["forecasts"] = nlohmann::json::array();
                    formatted["forecasts"][0] = forecast;
                    formatted["forecasts"][0]["casts"] = nlohmann::json::array();
                    formatted["forecasts"][0]["casts"].push_back(cast);

                    auto weather_formatted = weatherService_->formatWeatherData(formatted);
                    formattedForecast.push_back(weather_formatted);
                }
            }
        }

        nlohmann::json response_data;
        response_data["forecast"] = formattedForecast;

        // City info
        nlohmann::json city_info;
        if (forecastData.contains("forecasts") && !forecastData["forecasts"].empty()) {
            city_info["name"] = forecastData["forecasts"][0].value("city", "未知");
        }
        response_data["city"] = city_info;

        return utils::HttpResponse::createSuccessResponse(response_data, "获取天气预报成功");

    } catch (const std::exception& e) {
        std::cerr << "[WeatherController] Error getting forecast: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("获取天气预报失败");
    }
}

http::response<http::string_body> WeatherController::checkFlightSafety(
    const http::request<http::string_body>& req
) {
    std::cout << "[WeatherController] GET /api/v1/weather/flight-safety" << std::endl;

    try {
        // 验证JWT Token
        auto user_id_opt = validateTokenAndGetUserId(req);
        if (!user_id_opt.has_value()) {
            return utils::HttpResponse::createUnauthorizedResponse("需要登录检查飞行安全");
        }

        // 解析查询参数
        auto params = parseQueryParams(req);

        if (params.find("lat") == params.end() || params.find("lon") == params.end()) {
            return utils::HttpResponse::createErrorResponse("缺少坐标参数");
        }

        double lat = std::stod(params["lat"]);
        double lon = std::stod(params["lon"]);

        // 获取当前天气
        auto weatherData = weatherService_->getCurrentWeatherByCoords(lat, lon);

        // 检查飞行安全性
        auto safetyCheck = weatherService_->checkFlightSafety(weatherData);

        // 格式化天气数据
        auto formattedWeather = weatherService_->formatWeatherData(weatherData);

        nlohmann::json response_data;
        response_data["safety"] = safetyCheck;
        response_data["weather"] = formattedWeather;
        response_data["coordinates"] = {
            {"lat", lat},
            {"lon", lon}
        };

        std::string message = safetyCheck["safe"].get<bool>() ?
            "天气条件适合飞行" : "天气条件不适合飞行";

        return utils::HttpResponse::createSuccessResponse(response_data, message);

    } catch (const std::exception& e) {
        std::cerr << "[WeatherController] Error checking flight safety: " << e.what() << std::endl;
        return utils::HttpResponse::createInternalErrorResponse("检查飞行安全失败");
    }
}

} // namespace controllers
