/**
 * @file User.cpp
 * @brief Implementation of User Model
 */

#include "User.h"
#include <algorithm>
#include <sstream>

namespace study_planner {

User::User() : m_id(0), m_username(""), m_passwordHash(""), m_createdAt("") {}

User::User(int id, const std::string& username, const std::string& passwordHash)
    : m_id(id), m_username(username), m_passwordHash(passwordHash), m_createdAt("") {}

User::~User() {}

// Getters
int User::getId() const { return m_id; }
std::string User::getUsername() const { return m_username; }
std::string User::getPasswordHash() const { return m_passwordHash; }
std::string User::getCreatedAt() const { return m_createdAt; }

// Setters
void User::setId(int id) { m_id = id; }
void User::setUsername(const std::string& username) { m_username = username; }
void User::setPasswordHash(const std::string& passwordHash) { m_passwordHash = passwordHash; }
void User::setCreatedAt(const std::string& createdAt) { m_createdAt = createdAt; }

std::map<std::string, std::string> User::toMap() const {
    return {
        {"id", std::to_string(m_id)},
        {"username", m_username},
        {"password_hash", m_passwordHash},
        {"created_at", m_createdAt}
    };
}

User User::fromMap(const std::map<std::string, std::string>& data) {
    User user;
    if (data.count("id")) user.setId(std::stoi(data.at("id")));
    if (data.count("username")) user.setUsername(data.at("username"));
    if (data.count("password_hash")) user.setPasswordHash(data.at("password_hash"));
    if (data.count("created_at")) user.setCreatedAt(data.at("created_at"));
    return user;
}

// Simple hash function for demo purposes (NOT secure for production)
std::string User::hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

bool User::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

} // namespace study_planner
