#pragma once

#include <memory>
#include <vector>
#include "user.h"
#include <mysql/mysql.h>

/**
 * @brief 用户数据访问对象
 * 
 * 负责用户数据的持久化操作
 */
class UserDAO {
private:
    MYSQL* mysql;

public:
    /**
     * @brief 构造函数
     * @param mysql MySQL连接
     */
    UserDAO(MYSQL* mysql);
    
    /**
     * @brief 根据ID查询用户
     * @param id 用户ID
     * @return 用户对象指针，未找到返回nullptr
     */
    std::shared_ptr<User> findById(int id);
    
    /**
     * @brief 根据用户名查询用户
     * @param username 用户名
     * @return 用户对象指针，未找到返回nullptr
     */
    std::shared_ptr<User> findByUsername(const std::string& username);
    
    /**
     * @brief 查询所有用户
     * @return 用户对象列表
     */
    std::vector<std::shared_ptr<User>> findAll();
    
    /**
     * @brief 插入新用户
     * @param user 用户对象
     * @return 插入成功返回true，否则返回false
     */
    bool insert(const std::shared_ptr<User>& user);
    
    /**
     * @brief 更新用户信息
     * @param user 用户对象
     * @return 更新成功返回true，否则返回false
     */
    bool update(const std::shared_ptr<User>& user);
    
    /**
     * @brief 删除用户
     * @param id 用户ID
     * @return 删除成功返回true，否则返回false
     */
    bool remove(int id);
};
