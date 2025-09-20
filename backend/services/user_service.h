#ifndef SERVICES_USER_SERVICE_H
#define SERVICES_USER_SERVICE_H

#include "../models/user.h"
#include "../models/user_dao.h"
#include "../utils/password.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace services {
    class UserService {
    private:
        std::shared_ptr<UserDAO> user_dao_;

    public:
        explicit UserService(std::shared_ptr<UserDAO> user_dao) : user_dao_(user_dao) {}

        // User management
        std::optional<User> create_user(const std::string& username, const std::string& email,
                                               const std::string& password, const std::string& first_name,
                                               const std::string& last_name, const std::string& phone = "");

        std::optional<User> get_user_by_id(int id);
        std::optional<User> get_user_by_username(const std::string& username);
        std::optional<User> get_user_by_email(const std::string& email);
        std::vector<User> get_all_users();

        bool update_user(const User& user);
        bool delete_user(int id);

        // Authentication
        std::optional<User> authenticate_user(const std::string& username, const std::string& password);
        bool update_last_login(int user_id);

        // Helper methods
        bool is_username_available(const std::string& username);
        bool is_email_available(const std::string& email);
        bool verify_password(const std::string& password, const std::string& hash);
    };
}

#endif // SERVICES_USER_SERVICE_H