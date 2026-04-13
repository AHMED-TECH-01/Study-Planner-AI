/**
 * @file ProgressController.cpp
 * @brief Implementation of Progress Controller
 */

#include "ProgressController.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace study_planner {

ProgressController::ProgressController() {}

ProgressController::~ProgressController() {}

std::map<std::string, std::string> ProgressController::handleGetProgress(int userId) {
    ProgressStats stats = m_progressTracker.getProgressStats(userId);

    std::map<std::string, std::string> result;
    result["total_hours_studied"] = std::to_string(stats.totalHoursStudied);
    result["completion_percentage"] = std::to_string(stats.completionPercentage);
    result["sessions_completed"] = std::to_string(stats.sessionsCompleted);
    result["total_sessions"] = std::to_string(stats.totalSessions);

    // Add course progress
    int i = 0;
    for (const auto& cp : stats.courseProgress) {
        result["course_" + std::to_string(i) + "_name"] = cp.first;
        result["course_" + std::to_string(i) + "_progress"] = std::to_string(cp.second);
        i++;
    }
    result["course_count"] = std::to_string(i);

    return result;
}

std::map<std::string, std::string> ProgressController::handleLogProgress(int userId, const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    // Validate required fields
    if (!requestData.count("session_id") || !requestData.count("course_id")) {
        response["status"] = "error";
        response["message"] = "Missing required fields";
        return response;
    }

    int sessionId = std::stoi(requestData.at("session_id"));
    int courseId = std::stoi(requestData.count("course_id") ? requestData.at("course_id") : "0");
    int topicId = requestData.count("topic_id") ? std::stoi(requestData.at("topic_id")) : 0;
    double hoursStudied = requestData.count("hours_studied") ?
        std::stod(requestData.at("hours_studied")) : 1.0;
    double completion = requestData.count("completion") ?
        std::stod(requestData.at("completion")) : 100.0;

    if (m_progressTracker.updateProgress(userId, sessionId, courseId, topicId, hoursStudied, completion)) {
        response["status"] = "success";
        response["message"] = "Progress logged successfully";
    } else {
        response["status"] = "error";
        response["message"] = "Failed to log progress";
    }

    return response;
}

std::map<std::string, std::string> ProgressController::handleGetCourseProgress(int userId, int courseId) {
    return m_progressTracker.getCourseProgress(userId, courseId);
}

std::map<std::string, std::string> ProgressController::handleGetDailyStats(int userId, const std::string& date) {
    return m_progressTracker.getDailyStats(userId, date);
}

} // namespace study_planner
