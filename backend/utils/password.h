#ifndef UTILS_PASSWORD_H
#define UTILS_PASSWORD_H

#include <string>

namespace utils {
    class PasswordHasher {
    public:
        static std::string hash_password(const std::string& password);
        static bool verify_password(const std::string& password, const std::string& hash);
        static std::string generate_salt();
    };
}

#endif // UTILS_PASSWORD_H