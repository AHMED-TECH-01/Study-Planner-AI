/**
 * @file User.h
 * @brief User Model - Represents a student user
 *
 * This class encapsulates user data and authentication logic.
 * Uses encapsulation to protect sensitive user information.
 */

#ifndef USER_H
#define USER_H

#include <string>
#include <map>

namespace study_planner {

/**
 * @class User
 * @brief Represents a student user in the system
 *
 * Handles user data including credentials and profile information.
 * Provides methods for user registration and authentication.
 */
class User {
public:
    /**
     * @brief Default constructor
     */
    User();

    /**
     * @brief Parameterized constructor
     * @param id User ID
     * @param username User's username
     * @param passwordHash Hashed password
     */
    User(int id, const std::string& username, const std::string& passwordHash);

    /**
     * @brief Destructor
     */
    ~User();

    // Getters
    int getId() const;
    std::string getUsername() const;
    std::string getPasswordHash() const;
    std::string getCreatedAt() const;

    // Setters
    void setId(int id);
    void setUsername(const std::string& username);
    void setPasswordHash(const std::string& passwordHash);
    void setCreatedAt(const std::string& createdAt);

    /**
     * @brief Convert user to map for JSON serialization
     * @return Map containing user data
     */
    std::map<std::string, std::string> toMap() const;

    /**
     * @brief Create User object from database result
     * @param data Map containing user data from database
     * @return User object
     */
    static User fromMap(const std::map<std::string, std::string>& data);

    /**
     * @brief Hash password using simple hashing (for demo purposes)
     * @param password Plain text password
     * @return Hashed password
     */
    static std::string hashPassword(const std::string& password);

    /**
     * @brief Verify password against hash
     * @param password Plain text password
     * @param hash Stored password hash
     * @return true if password matches
     */
    static bool verifyPassword(const std::string& password, const std::string& hash);

private:
    int m_id;
    std::string m_username;
    std::string m_passwordHash;
    std::string m_createdAt;
};

} // namespace study_planner

#endif // USER_H
