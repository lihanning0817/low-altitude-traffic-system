#include "user_dao.h"
#include <iostream>

UserDAO::UserDAO(MYSQL* mysql) : mysql(mysql) {}

std::shared_ptr<User> UserDAO::findById(int id) {
    try {
        // 构建SQL查询
        std::string sql = "SELECT id, username, password_hash, role, created_at FROM users WHERE id = " + std::to_string(id);
        
        // 执行查询
        if (mysql_query(mysql, sql.c_str())) {
            std::cerr << "查询用户失败: " << mysql_error(mysql) << std::endl;
            return nullptr;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(mysql);
        if (!result) {
            std::cerr << "获取结果集失败: " << mysql_error(mysql) << std::endl;
            return nullptr;
        }
        
        // 检查是否有结果
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            std::shared_ptr<User> user = std::make_shared<User>(
                std::stoi(row[0]),
                row[1] ? row[1] : "",
                row[2] ? row[2] : "",
                row[3] ? row[3] : "",
                row[4] ? row[4] : ""
            );
            
            // 释放结果集
            mysql_free_result(result);
            return user;
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "查询用户失败: " << e.what() << std::endl;
    }
    return nullptr;
}

std::shared_ptr<User> UserDAO::findByUsername(const std::string& username) {
    try {
        // 构建SQL查询
        // 准备转义字符串
        char* escaped_username = new char[username.length() * 2 + 1];
        mysql_real_escape_string(mysql, escaped_username, username.c_str(), username.length());
        std::string sql = "SELECT id, username, password_hash, role, created_at FROM users WHERE username = '" + std::string(escaped_username) + "'";
        delete[] escaped_username;
        
        // 执行查询
        if (mysql_query(mysql, sql.c_str())) {
            std::cerr << "根据用户名查询用户失败: " << mysql_error(mysql) << std::endl;
            return nullptr;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(mysql);
        if (!result) {
            std::cerr << "获取结果集失败: " << mysql_error(mysql) << std::endl;
            return nullptr;
        }
        
        // 检查是否有结果
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row) {
            std::shared_ptr<User> user = std::make_shared<User>(
                std::stoi(row[0]),
                row[1] ? row[1] : "",
                row[2] ? row[2] : "",
                row[3] ? row[3] : "",
                row[4] ? row[4] : ""
            );
            
            // 释放结果集
            mysql_free_result(result);
            return user;
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "根据用户名查询用户失败: " << e.what() << std::endl;
    }
    return nullptr;
}

std::vector<std::shared_ptr<User>> UserDAO::findAll() {
    std::vector<std::shared_ptr<User>> users;
    try {
        // 执行查询
        if (mysql_query(mysql, "SELECT id, username, password_hash, role, created_at FROM users")) {
            std::cerr << "查询所有用户失败: " << mysql_error(mysql) << std::endl;
            return users;
        }
        
        // 获取结果集
        MYSQL_RES* result = mysql_store_result(mysql);
        if (!result) {
            std::cerr << "获取结果集失败: " << mysql_error(mysql) << std::endl;
            return users;
        }
        
        // 遍历结果集
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            users.push_back(std::make_shared<User>(
                std::stoi(row[0]),
                row[1] ? row[1] : "",
                row[2] ? row[2] : "",
                row[3] ? row[3] : "",
                row[4] ? row[4] : ""
            ));
        }
        
        // 释放结果集
        mysql_free_result(result);
    } catch (const std::exception& e) {
        std::cerr << "查询所有用户失败: " << e.what() << std::endl;
    }
    return users;
}

bool UserDAO::insert(const std::shared_ptr<User>& user) {
    try {
        // 构建SQL查询
        std::string sql = "INSERT INTO users (username, password_hash, role, created_at) VALUES ('" +
                          mysql_real_escape_string(mysql, new char[user->getUsername().length() * 2 + 1], user->getUsername().c_str(), user->getUsername().length()) + "', '" +
                          mysql_real_escape_string(mysql, new char[user->getPasswordHash().length() * 2 + 1], user->getPasswordHash().c_str(), user->getPasswordHash().length()) + "', '" +
                          mysql_real_escape_string(mysql, new char[user->getRole().length() * 2 + 1], user->getRole().c_str(), user->getRole().length()) + "', NOW())";
        
        // 执行查询
        if (mysql_query(mysql, sql.c_str())) {
            std::cerr << "插入用户失败: " << mysql_error(mysql) << std::endl;
            return false;
        }
        
        // 检查是否插入成功
        return mysql_affected_rows(mysql) > 0;
    } catch (const std::exception& e) {
        std::cerr << "插入用户失败: " << e.what() << std::endl;
        return false;
    }
}

bool UserDAO::update(const std::shared_ptr<User>& user) {
    try {
        // 构建SQL查询
        std::string sql = "UPDATE users SET username = '" +
                          mysql_real_escape_string(mysql, new char[user->getUsername().length() * 2 + 1], user->getUsername().c_str(), user->getUsername().length()) + "', " +
                          "password_hash = '" +
                          mysql_real_escape_string(mysql, new char[user->getPasswordHash().length() * 2 + 1], user->getPasswordHash().c_str(), user->getPasswordHash().length()) + "', " +
                          "role = '" +
                          mysql_real_escape_string(mysql, new char[user->getRole().length() * 2 + 1], user->getRole().c_str(), user->getRole().length()) + "' " +
                          "WHERE id = " + std::to_string(user->getId());
        
        // 执行查询
        if (mysql_query(mysql, sql.c_str())) {
            std::cerr << "更新用户失败: " << mysql_error(mysql) << std::endl;
            return false;
        }
        
        // 检查是否更新成功
        return mysql_affected_rows(mysql) > 0;
    } catch (const std::exception& e) {
        std::cerr << "更新用户失败: " << e.what() << std::endl;
        return false;
    }
}

bool UserDAO::remove(int id) {
    try {
        // 构建SQL查询
        std::string sql = "DELETE FROM users WHERE id = " + std::to_string(id);
        
        // 执行查询
        if (mysql_query(mysql, sql.c_str())) {
            std::cerr << "删除用户失败: " << mysql_error(mysql) << std::endl;
            return false;
        }
        
        // 检查是否删除成功
        return mysql_affected_rows(mysql) > 0;
    } catch (const std::exception& e) {
        std::cerr << "删除用户失败: " << e.what() << std::endl;
        return false;
    }
}
