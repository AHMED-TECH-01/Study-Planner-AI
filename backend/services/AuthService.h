/**
 * @file AuthService.h
 * @brief Authentication Service - Handles user registration and login
 *
 * This class manages user authentication including registration,
 * login, and session management.
 */

#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include <map>
#include "../models/User.h"

namespace study_planner {

/**
 * @class AuthService
 * @brief Handles user authentication operations
 *
 * Provides methods for user registration and login with
 * password hashing and validation.
 */
class AuthService {
public:
    /**
     * @brief Default constructor
     */
    AuthService();

    /**
     * @brief Destructor
     */
    ~AuthService();

    /**
     * @brief Register a new user
     * @param username User's username
     * @param password User's password
     * @return User object if successful, empty User if failed
     */
    User registerUser(const std::string& username, const std::string& password);

    /**
     * @brief Login an existing user
     * @param username User's username
     * @param password User's password
     * @return User object if successful, empty User if failed
     */
    User loginUser(const std::string& username, const std::string& password);

    /**
     * @brief Validate user credentials
     * @param username Username
     * @param password Password
     * @return true if valid
     */
    bool validateCredentials(const std::string& username, const std::string& password);

    /**
     * @brief Check if username exists
     * @param username Username to check
     * @return true if exists
     */
    bool usernameExists(const std::string& username);

    /**
     * @brief Get user by ID
     * @param userId User ID
     * @return User object
     */
    User getUserById(int userId);

    /**
     * @brief Get user by username
     * @param username Username
     * @return User object
     */
    User getUserByUsername(const std::string& username);

private:
    /**
     * @brief Hash password for storage
     * @param password Plain text password
     * @return Hashed password
     */
    std::string hashPassword(const std::string& password);

    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Stored hash
     * @return true if match
     */
    bool verifyPassword(const std::string& password, const std::string& hash);
};

} // namespace study_planner

#endif // AUTH_SERVICE_H
