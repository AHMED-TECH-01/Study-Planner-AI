/**
 * @file AuthController.h
 * @brief Authentication Controller - Handles auth endpoints
 *
 * This controller handles user registration and login endpoints.
 */

#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <string>
#include <map>
#include "../models/User.h"
#include "../services/AuthService.h"

namespace study_planner {

/**
 * @class AuthController
 * @brief Handles authentication HTTP endpoints
 */
class AuthController {
public:
    /**
     * @brief Default constructor
     */
    AuthController();

    /**
     * @brief Destructor
     */
    ~AuthController();

    /**
     * @brief Handle user registration
     * @param requestData Map containing username and password
     * @return Response map with status and user data
     */
    std::map<std::string, std::string> handleRegister(const std::map<std::string, std::string>& requestData);

    /**
     * @brief Handle user login
     * @param requestData Map containing username and password
     * @return Response map with status and user data
     */
    std::map<std::string, std::string> handleLogin(const std::map<std::string, std::string>& requestData);

private:
    AuthService m_authService;
};

} // namespace study_planner

#endif // AUTH_CONTROLLER_H
