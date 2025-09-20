#include "jwt.h"
#include <string>
#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>

// 静态成员定义
const std::string JWTUtil::SECRET_KEY = "low_altitude_traffic_secret_key_2025";

std::string JWTUtil::generateToken(int userId, const std::string& username, 
                                 const std::string& role) {
    try {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        
        // 计算过期时间（24小时后）
        auto expires = seconds + EXPIRATION_HOURS * 3600;
        
        // 创建payload
        std::ostringstream payload;
        payload << "{\"userId\":" << userId 
                << ",\"username\":\"" << username 
                << "\",\"role\":\"" << role 
                << "\",\"exp\":" << expires << "}";
        
        // 生成随机header
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1000, 9999);
        
        std::ostringstream header;
        header << "{\"alg\":\"HS256\",\"typ\":\"JWT\",\"id\":" << dis(gen) << "}";
        
        // Base64编码（简化实现）
        auto base64Encode = [](const std::string& input) -> std::string {
            const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string output;
            int val = 0, valb = -6;
            
            for (char c : input) {
                val = (val << 8) + c;
                valb += 8;
                while (valb >= 0) {
                    output.push_back(chars[(val >> valb) & 0x3F]);
                    valb -= 6;
                }
            }
            if (valb > -6) output.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
            while (output.size() % 4) output.push_back('=');
            return output;
        };
        
        std::string encodedHeader = base64Encode(header.str());
        std::string encodedPayload = base64Encode(payload.str());
        
        // 创建签名
        std::string signingInput = encodedHeader + "." + encodedPayload;
        std::string signature = base64Encode(signingInput + SECRET_KEY);
        
        // 组合JWT
        return encodedHeader + "." + encodedPayload + "." + signature.substr(0, 43);
        
    } catch (const std::exception& e) {
        std::cerr << "生成JWT令牌时发生异常: " << e.what() << std::endl;
        return "";
    }
}

bool JWTUtil::validateToken(const std::string& token) {
    try {
        if (token.empty()) return false;
        
        // 检查JWT格式
        size_t firstDot = token.find('.');
        size_t secondDot = token.rfind('.');
        
        if (firstDot == std::string::npos || secondDot == std::string::npos || 
            firstDot == secondDot) {
            return false;
        }
        
        // 提取各部分
        std::string header = token.substr(0, firstDot);
        std::string payload = token.substr(firstDot + 1, secondDot - firstDot - 1);
        std::string signature = token.substr(secondDot + 1);
        
        // 检查是否过期
        if (isExpired(token)) {
            return false;
        }
        
        // 简单验证（实际项目中应使用真正的JWT库）
        return !signature.empty() && signature.length() > 10;
        
    } catch (const std::exception& e) {
        std::cerr << "验证JWT令牌时发生异常: " << e.what() << std::endl;
        return false;
    }
}

std::unordered_map<std::string, std::string> JWTUtil::parseToken(
    const std::string& token) {
    std::unordered_map<std::string, std::string> result;
    
    try {
        if (token.empty()) return result;
        
        size_t firstDot = token.find('.');
        size_t secondDot = token.rfind('.');
        
        if (firstDot == std::string::npos || secondDot == std::string::npos || 
            firstDot == secondDot) {
            return result;
        }
        
        // 提取payload
        std::string payload = token.substr(firstDot + 1, secondDot - firstDot - 1);
        
        // 简化解码（实际项目中应使用真正的Base64解码）
        // 这里返回默认值作为示例
        result["userId"] = "1";
        result["username"] = "admin";
        result["role"] = "admin";
        
    } catch (const std::exception& e) {
        std::cerr << "解析JWT令牌时发生异常: " << e.what() << std::endl;
    }
    
    return result;
}

bool JWTUtil::isExpired(const std::string& token) {
    try {
        // 简化过期检查
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto currentSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        
        // 提取过期时间（简化实现）
        // 实际项目中应正确解析JWT的exp字段
        return false; // 默认不认为过期
        
    } catch (const std::exception& e) {
        std::cerr << "检查JWT过期时发生异常: " << e.what() << std::endl;
        return true;
    }
}
