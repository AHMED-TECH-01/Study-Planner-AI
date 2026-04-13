/**
 * @file AuthService.cpp
 * @brief Implementation of Authentication Service
 */

#include "AuthService.h"
#include "../database/db_manager.h"
#include <iostream>
#include <sstream>

namespace study_planner {

AuthService::AuthService() {}

AuthService::~AuthService() {}

User AuthService::registerUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return User();
    }

    if (usernameExists(username)) {
        return User(); // Username already exists
    }

    std::string hashedPassword = hashPassword(password);

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "INSERT INTO users (username, password_hash) VALUES ('" << username << "', '" << hashedPassword << "')";

    if (db.executeQuery(oss.str())) {
        int userId = db.getLastInsertRowId();
        return User(userId, username, hashedPassword);
    }

    return User();
}

User AuthService::loginUser(const std::string& username, const std::string& password) {
    User user = getUserByUsername(username);

    if (user.getId() == 0) {
        return User(); // User not found
    }

    if (verifyPassword(password, user.getPasswordHash())) {
        return user;
    }

    return User(); // Invalid password
}

bool AuthService::validateCredentials(const std::string& username, const std::string& password) {
    User user = loginUser(username, password);
    return user.getId() > 0;
}

bool AuthService::usernameExists(const std::string& username) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT COUNT(*) as count FROM users WHERE username = '" << username << "'";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return results[0].at("count") != "0";
    }

    return false;
}

User AuthService::getUserById(int userId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT * FROM users WHERE id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return User::fromMap(results[0]);
    }

    return User();
}

User AuthService::getUserByUsername(const std::string& username) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT * FROM users WHERE username = '" << username << "'";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return User::fromMap(results[0]);
    }

    return User();
}

std::string AuthService::hashPassword(const std::string& password) {
    // Simple hash for demo purposes (NOT secure for production)
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

bool AuthService::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

} // namespace study_planner
