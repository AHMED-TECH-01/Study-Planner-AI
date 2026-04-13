/**
 * @file AuthController.cpp
 * @brief Implementation of Authentication Controller
 */

#include "AuthController.h"
#include <iostream>

namespace study_planner {

AuthController::AuthController() {}

AuthController::~AuthController() {}

std::map<std::string, std::string> AuthController::handleRegister(const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    auto it = requestData.find("username");
    if (it == requestData.end() || it->second.empty()) {
        response["status"] = "error";
        response["message"] = "Username is required";
        return response;
    }

    it = requestData.find("password");
    if (it == requestData.end() || it->second.empty()) {
        response["status"] = "error";
        response["message"] = "Password is required";
        return response;
    }

    std::string username = requestData.at("username");
    std::string password = requestData.at("password");

    User user = m_authService.registerUser(username, password);

    if (user.getId() == 0) {
        response["status"] = "error";
        response["message"] = "Registration failed - username may already exist";
        return response;
    }

    response["status"] = "success";
    response["message"] = "User registered successfully";
    response["user_id"] = std::to_string(user.getId());
    response["username"] = user.getUsername();

    return response;
}

std::map<std::string, std::string> AuthController::handleLogin(const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    auto it = requestData.find("username");
    if (it == requestData.end() || it->second.empty()) {
        response["status"] = "error";
        response["message"] = "Username is required";
        return response;
    }

    it = requestData.find("password");
    if (it == requestData.end() || it->second.empty()) {
        response["status"] = "error";
        response["message"] = "Password is required";
        return response;
    }

    std::string username = requestData.at("username");
    std::string password = requestData.at("password");

    User user = m_authService.loginUser(username, password);

    if (user.getId() == 0) {
        response["status"] = "error";
        response["message"] = "Invalid username or password";
        return response;
    }

    response["status"] = "success";
    response["message"] = "Login successful";
    response["user_id"] = std::to_string(user.getId());
    response["username"] = user.getUsername();

    return response;
}

} // namespace study_planner
