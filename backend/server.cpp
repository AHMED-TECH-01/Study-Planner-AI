
#include <iostream>
#include <string>
#include <map>
#include <crow.h>
#include "database/db_manager.h"
#include "controllers/AuthController.h"
#include "controllers/CourseController.h"
#include "controllers/ScheduleController.h"
#include "controllers/ProgressController.h"

// Simple JWT-like token generation (for demo purposes)
std::string generateToken(int userId) {
    return "token_user_" + std::to_string(userId) + "_" + std::to_string(time(nullptr));
}

// Parse JSON body from request
std::map<std::string, std::string> parseJsonBody(const std::string& body) {
    std::map<std::string, std::string> data;
    std::string key, value;
    bool inKey = true;
    bool inString = false;
    bool skipColon = false;

    for (size_t i = 0; i < body.size(); i++) {
        char c = body[i];

        if (c == '"' && (i == 0 || body[i-1] != '\\')) {
            inString = !inString;
            continue;
        }

        if (!inString) {
            if (c == '{' || c == '}' || c == ',' || c == ' ') continue;
            if (c == ':') {
                skipColon = true;
                inKey = false;
                continue;
            }
        }

        if (inString) {
            if (inKey) key += c;
            else value += c;
        }

        if (!inString && skipColon && (c == '"' || isalnum(c))) {
            skipColon = false;
            if (!key.empty() && !value.empty()) {
                data[key] = value;
                key.clear();
                value.clear();
                inKey = true;
            }
        }
    }

    // Last pair
    if (!key.empty() && !value.empty()) {
        data[key] = value;
    }

    return data;
}

// Get user ID from request (simplified - in production use proper JWT)
int getUserIdFromRequest(const crow::request& req) {
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty()) return 0;

    // Simple token parsing
    std::string token = authHeader;
    if (token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }

    // Extract user ID from token (format: token_user_X_timestamp)
    if (token.substr(0, 11) == "token_user_") {
        size_t underscore = token.find('_', 11);
        if (underscore != std::string::npos) {
            try {
                return std::stoi(token.substr(11, underscore - 11));
            } catch (...) {}
        }
    }

    return 0;
}

int main() {
    // Initialize database
    study_planner::DBManager& db = study_planner::DBManager::getInstance();
    if (!db.initialize("study_planner.db")) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }

    // Initialize controllers
    study_planner::AuthController authController;
    study_planner::CourseController courseController;
    study_planner::ScheduleController scheduleController;
    study_planner::ProgressController progressController;

    crow::App<crow::CORSHandler> app;

    // Configure CORS
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global();

    // Simple route for CORS preflight
    CROW_ROUTE(app, "/health")
    ([]() {
        return crow::response(200, "{\"status\":\"ok\"}");
    });

    // POST /register
    CROW_ROUTE(app, "/register").methods(crow::Method::POST)
    ([&authController](const crow::request& req) {
        auto body = parseJsonBody(req.body);
        auto result = authController.handleRegister(body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        if (result["status"] == "success") {
            jsonResponse["user_id"] = std::stoi(result["user_id"]);
            jsonResponse["username"] = result["username"];
            jsonResponse["token"] = generateToken(std::stoi(result["user_id"]));
        }

        return crow::response(jsonResponse.dump());
    });

    // POST /login
    CROW_ROUTE(app, "/login").methods(crow::Method::POST)
    ([&authController](const crow::request& req) {
        auto body = parseJsonBody(req.body);
        auto result = authController.handleLogin(body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        if (result["status"] == "success") {
            jsonResponse["user_id"] = std::stoi(result["user_id"]);
            jsonResponse["username"] = result["username"];
            jsonResponse["token"] = generateToken(std::stoi(result["user_id"]));
        }

        return crow::response(jsonResponse.dump());
    });

    // GET /courses
    CROW_ROUTE(app, "/courses").methods(crow::Method::GET)
    ([&courseController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto courses = courseController.handleGetCourses(userId);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = "success";
        jsonResponse["courses"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& course : courses) {
            jsonResponse["courses"][i] = crow::json::wvalue::list();
            for (const auto& kv : course) {
                try {
                    jsonResponse["courses"][i][kv.first] = std::stod(kv.second);
                } catch (...) {
                    jsonResponse["courses"][i][kv.first] = kv.second;
                }
            }
            i++;
        }

        return crow::response(jsonResponse.dump());
    });

    // POST /courses
    CROW_ROUTE(app, "/courses").methods(crow::Method::POST)
    ([&courseController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto body = parseJsonBody(req.body);
        auto result = courseController.handleAddCourse(userId, body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        if (result["status"] == "success" && result.count("course_id")) {
            jsonResponse["course_id"] = std::stoi(result["course_id"]);
        }

        return crow::response(jsonResponse.dump());
    });

    // DELETE /courses/:id
    CROW_ROUTE(app, "/courses/<int>").methods(crow::Method::DELETE)
    ([&courseController](const crow::request& req, int courseId) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto result = courseController.handleRemoveCourse(userId, courseId);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        return crow::response(jsonResponse.dump());
    });

    // GET /courses/:id/topics
    CROW_ROUTE(app, "/courses/<int>/topics").methods(crow::Method::GET)
    ([&courseController](const crow::request& req, int courseId) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto topics = courseController.handleGetTopics(userId, courseId);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = "success";
        jsonResponse["topics"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& topic : topics) {
            for (const auto& kv : topic) {
                try {
                    jsonResponse["topics"][i][kv.first] = std::stod(kv.second);
                } catch (...) {
                    jsonResponse["topics"][i][kv.first] = kv.second;
                }
            }
            i++;
        }

        return crow::response(jsonResponse.dump());
    });

    // POST /courses/:id/topics
    CROW_ROUTE(app, "/courses/<int>/topics").methods(crow::Method::POST)
    ([&courseController](const crow::request& req, int courseId) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto body = parseJsonBody(req.body);
        auto result = courseController.handleAddTopic(userId, courseId, body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        return crow::response(jsonResponse.dump());
    });

    // GET /schedule
    CROW_ROUTE(app, "/schedule").methods(crow::Method::GET)
    ([&scheduleController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        std::string startDate, endDate;
        auto queryParams = req.url_params;
        if (queryParams.keys().count("start_date")) {
            startDate = queryParams.get("start_date");
        }
        if (queryParams.keys().count("end_date")) {
            endDate = queryParams.get("end_date");
        }

        auto schedule = scheduleController.handleGetSchedule(userId, startDate, endDate);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = "success";
        jsonResponse["sessions"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& session : schedule) {
            for (const auto& kv : session) {
                try {
                    jsonResponse["sessions"][i][kv.first] = std::stod(kv.second);
                } catch (...) {
                    jsonResponse["sessions"][i][kv.first] = kv.second;
                }
            }
            i++;
        }

        return crow::response(jsonResponse.dump());
    });

    // POST /schedule/generate
    CROW_ROUTE(app, "/schedule/generate").methods(crow::Method::POST)
    ([&scheduleController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto body = parseJsonBody(req.body);
        auto result = scheduleController.handleGenerateSchedule(userId, body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        if (result["status"] == "success" && result.count("sessions_count")) {
            jsonResponse["sessions_count"] = std::stoi(result["sessions_count"]);
        }

        return crow::response(jsonResponse.dump());
    });

    // PUT /schedule/:id
    CROW_ROUTE(app, "/schedule/<int>").methods(crow::Method::PUT)
    ([&scheduleController](const crow::request& req, int sessionId) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto body = parseJsonBody(req.body);
        auto result = scheduleController.handleUpdateSession(userId, sessionId, body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        return crow::response(jsonResponse.dump());
    });

    // GET /recommendations
    CROW_ROUTE(app, "/recommendations").methods(crow::Method::GET)
    ([&scheduleController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto recommendations = scheduleController.handleGetRecommendations(userId);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = "success";
        jsonResponse["recommendations"] = crow::json::wvalue::list();

        for (size_t i = 0; i < recommendations.size(); i++) {
            jsonResponse["recommendations"][i] = recommendations[i];
        }

        return crow::response(jsonResponse.dump());
    });

    // GET /progress
    CROW_ROUTE(app, "/progress").methods(crow::Method::GET)
    ([&progressController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto stats = progressController.handleGetProgress(userId);

        crow::json::wvalue jsonResponse;
        for (const auto& kv : stats) {
            try {
                jsonResponse[kv.first] = std::stod(kv.second);
            } catch (...) {
                jsonResponse[kv.first] = kv.second;
            }
        }

        return crow::response(jsonResponse.dump());
    });

    // POST /progress
    CROW_ROUTE(app, "/progress").methods(crow::Method::POST)
    ([&progressController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        auto body = parseJsonBody(req.body);
        auto result = progressController.handleLogProgress(userId, body);

        crow::json::wvalue jsonResponse;
        jsonResponse["status"] = result["status"];
        jsonResponse["message"] = result["message"];

        return crow::response(jsonResponse.dump());
    });

    // GET /progress/daily
    CROW_ROUTE(app, "/progress/daily").methods(crow::Method::GET)
    ([&progressController](const crow::request& req) {
        int userId = getUserIdFromRequest(req);
        if (userId == 0) {
            return crow::response(401, "{\"status\":\"error\",\"message\":\"Unauthorized\"}");
        }

        std::string date;
        auto queryParams = req.url_params;
        if (queryParams.keys().count("date")) {
            date = queryParams.get("date");
        }

        auto stats = progressController.handleGetDailyStats(userId, date);

        crow::json::wvalue jsonResponse;
        for (const auto& kv : stats) {
            try {
                jsonResponse[kv.first] = std::stod(kv.second);
            } catch (...) {
                jsonResponse[kv.first] = kv.second;
            }
        }

        return crow::response(jsonResponse.dump());
    });

    std::cout << "Server starting on http://0.0.0.0:18080" << std::endl;
    app.port(18080).multithreaded().run();

    return 0;
}
