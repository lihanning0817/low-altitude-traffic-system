#include "user_service.h"
#include "../utils/password.h"
#include <stdexcept>
#include <iostream>

namespace services {
    std::optional<models::User> UserService::create_user(const std::string& username, const std::string& email,
                                                        const std::string& password, const std::string& first_name,
                                                        const std::string& last_name, const std::string& phone) {
        // Check if username or email already exists
        if (!is_username_available(username)) {
            throw std::invalid_argument("Username already exists");
        }

        if (!is_email_available(email)) {
            throw std::invalid_argument("Email already exists");
        }

        // Hash the password
        std::string password_hash = utils::PasswordHasher::hash_password(password);

        // Create user object
        models::User user;
        user.username = username;
        user.email = email;
        user.password_hash = password_hash;
        user.role = models::UserRole::USER;
        user.first_name = first_name;
        user.last_name = last_name;
        user.phone = phone;
        user.is_active = true;

        // Save to database
        int user_id = user_dao_->create_user(user);
        if (user_id > 0) {
            user.id = user_id;
            return user;
        }

        return std::nullopt;
    }

    std::optional<models::User> UserService::get_user_by_id(int id) {
        return user_dao_->get_user_by_id(id);
    }

    std::optional<models::User> UserService::get_user_by_username(const std::string& username) {
        return user_dao_->get_user_by_username(username);
    }

    std::optional<models::User> UserService::get_user_by_email(const std::string& email) {
        return user_dao_->get_user_by_email(email);
    }

    std::vector<models::User> UserService::get_all_users() {
        return user_dao_->get_all_users();
    }

    bool UserService::update_user(const models::User& user) {
        return user_dao_->update_user(user);
    }

    bool UserService::delete_user(int id) {
        return user_dao_->delete_user(id);
    }

    std::optional<models::User> UserService::authenticate_user(const std::string& username, const std::string& password) {
        auto user = user_dao_->authenticate_user(username, password);
        if (user.has_value()) {
            // In a real implementation, you would verify the password hash here
            // For now, we'll just return the user if authentication succeeds
            return user;
        }
        return std::nullopt;
    }

    bool UserService::update_last_login(int user_id) {
        return user_dao_->update_user_last_login(user_id);
    }

    bool UserService::is_username_available(const std::string& username) {
        auto user = user_dao_->get_user_by_username(username);
        return !user.has_value();
    }

    bool UserService::is_email_available(const std::string& email) {
        auto user = user_dao_->get_user_by_email(email);
        return !user.has_value();
    }


    bool UserService::verify_password(const std::string& password, const std::string& hash) {
        return utils::PasswordHasher::verify_password(password, hash);
    }
}