/**
 * @file CourseController.cpp
 * @brief Implementation of Course Controller
 */

#include "CourseController.h"
#include "../database/db_manager.h"
#include <iostream>
#include <sstream>

namespace study_planner {

CourseController::CourseController() {}

CourseController::~CourseController() {}

std::vector<std::map<std::string, std::string>> CourseController::handleGetCourses(int userId) {
    std::vector<std::map<std::string, std::string>> courses;

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT * FROM courses WHERE user_id = " << userId << " ORDER BY created_at DESC";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            courses.push_back(row);
        }
    }

    return courses;
}

std::map<std::string, std::string> CourseController::handleAddCourse(int userId, const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    if (!validateCourseData(requestData)) {
        response["status"] = "error";
        response["message"] = "Invalid course data";
        return response;
    }

    std::string name = requestData.at("name");
    int difficulty = std::stoi(requestData.at("difficulty"));
    std::string examDate = requestData.count("exam_date") ? requestData.at("exam_date") : "";

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "INSERT INTO courses (user_id, name, difficulty, exam_date) VALUES ("
        << userId << ", '" << name << "', " << difficulty << ", ";
    if (examDate.empty()) {
        oss << "NULL)";
    } else {
        oss << "'" << examDate << "')";
    }

    if (db.executeQuery(oss.str())) {
        int courseId = db.getLastInsertRowId();

        // Add topics if provided
        if (requestData.count("topics")) {
            // Topics would be parsed from JSON - for now we add default topics
            oss.str("");
            oss << "INSERT INTO topics (course_id, name, estimated_hours) VALUES "
                << "(" << courseId << ", 'Chapter 1 - Basics', 2.0), "
                << "(" << courseId << ", 'Chapter 2 - Intermediate', 3.0), "
                << "(" << courseId << ", 'Chapter 3 - Advanced', 4.0)";
            db.executeQuery(oss.str());
        }

        response["status"] = "success";
        response["message"] = "Course added successfully";
        response["course_id"] = std::to_string(courseId);
    } else {
        response["status"] = "error";
        response["message"] = "Failed to add course";
    }

    return response;
}

std::map<std::string, std::string> CourseController::handleRemoveCourse(int userId, int courseId) {
    std::map<std::string, std::string> response;

    DBManager& db = DBManager::getInstance();

    // Verify course belongs to user
    std::ostringstream oss;
    oss << "SELECT COUNT(*) as count FROM courses WHERE id = " << courseId << " AND user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && results[0].at("count") == "0") {
        response["status"] = "error";
        response["message"] = "Course not found";
        return response;
    }

    // Delete course (cascades to topics and sessions)
    oss.str("");
    oss << "DELETE FROM courses WHERE id = " << courseId << " AND user_id = " << userId;

    if (db.executeQuery(oss.str())) {
        response["status"] = "success";
        response["message"] = "Course removed successfully";
    } else {
        response["status"] = "error";
        response["message"] = "Failed to remove course";
    }

    return response;
}

std::vector<std::map<std::string, std::string>> CourseController::handleGetTopics(int userId, int courseId) {
    std::vector<std::map<std::string, std::string>> topics;

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT t.* FROM topics t "
        << "JOIN courses c ON t.course_id = c.id "
        << "WHERE c.id = " << courseId << " AND c.user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            topics.push_back(row);
        }
    }

    return topics;
}

std::map<std::string, std::string> CourseController::handleAddTopic(int userId, int courseId, const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    // Verify course belongs to user
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT COUNT(*) as count FROM courses WHERE id = " << courseId << " AND user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && results[0].at("count") == "0") {
        response["status"] = "error";
        response["message"] = "Course not found";
        return response;
    }

    auto it = requestData.find("name");
    if (it == requestData.end() || it->second.empty()) {
        response["status"] = "error";
        response["message"] = "Topic name is required";
        return response;
    }

    std::string name = it->second;
    double estimatedHours = requestData.count("estimated_hours") ?
        std::stod(requestData.at("estimated_hours")) : 1.0;

    oss.str("");
    oss << "INSERT INTO topics (course_id, name, estimated_hours) VALUES ("
        << courseId << ", '" << name << "', " << estimatedHours << ")";

    if (db.executeQuery(oss.str())) {
        response["status"] = "success";
        response["message"] = "Topic added successfully";
        response["topic_id"] = std::to_string(db.getLastInsertRowId());
    } else {
        response["status"] = "error";
        response["message"] = "Failed to add topic";
    }

    return response;
}

bool CourseController::validateCourseData(const std::map<std::string, std::string>& data) {
    if (!data.count("name") || data.at("name").empty()) {
        return false;
    }

    if (!data.count("difficulty")) {
        return false;
    }

    try {
        int difficulty = std::stoi(data.at("difficulty"));
        return difficulty >= 1 && difficulty <= 5;
    } catch (...) {
        return false;
    }
}

} // namespace study_planner
