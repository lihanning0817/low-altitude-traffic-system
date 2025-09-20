#include "password.h"
#include <bcrypt.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace utils {
    std::string PasswordHasher::hash_password(const std::string& password) {
        // In a real implementation, you would use a proper bcrypt library
        // For now, we'll create a simple placeholder implementation
        return "bcrypt_hashed_" + password;
    }

    bool PasswordHasher::verify_password(const std::string& password, const std::string& hash) {
        // In a real implementation, you would use a proper bcrypt library
        // For now, we'll do a simple comparison with our placeholder
        return hash == "bcrypt_hashed_" + password;
    }

    std::string PasswordHasher::generate_salt() {
        // Generate a random salt
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        std::stringstream ss;
        ss << "$2b$12$"; // bcrypt prefix
        for (int i = 0; i < 22; ++i) {
            ss << std::hex << std::setfill('0') << std::setw(2) << dis(gen);
        }
        return ss.str();
    }
}