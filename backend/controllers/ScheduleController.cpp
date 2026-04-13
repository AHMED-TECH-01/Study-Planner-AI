/**
 * @file ScheduleController.cpp
 * @brief Implementation of Schedule Controller
 */

#include "ScheduleController.h"
#include "../database/db_manager.h"
#include <iostream>
#include <sstream>

namespace study_planner {

ScheduleController::ScheduleController() {}

ScheduleController::~ScheduleController() {}

std::map<std::string, std::string> ScheduleController::handleGenerateSchedule(int userId, const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    ScheduleConfig config;
    config.userId = userId;
    config.dailyStudyHours = requestData.count("daily_study_hours") ?
        std::stoi(requestData.at("daily_study_hours")) : 4;
    config.sessionDurationMinutes = requestData.count("session_duration") ?
        std::stoi(requestData.at("session_duration")) : 60;
    config.daysAhead = requestData.count("days_ahead") ?
        std::stoi(requestData.at("days_ahead")) : 7;

    // Check if user has courses
    DBManager& db = DBManager::getInstance();
    std::ostringstream oss;
    oss << "SELECT COUNT(*) as count FROM courses WHERE user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && results[0].at("count") == "0") {
        response["status"] = "error";
        response["message"] = "No courses found. Please add courses first.";
        return response;
    }

    // Check if user has topics
    oss.str("");
    oss << "SELECT COUNT(*) as count FROM topics t "
        << "JOIN courses c ON t.course_id = c.id "
        << "WHERE c.user_id = " << userId;

    if (db.executeSelect(oss.str(), results) && results[0].at("count") == "0") {
        response["status"] = "error";
        response["message"] = "No topics found. Please add topics to your courses.";
        return response;
    }

    // Generate schedule
    auto sessions = m_scheduler.generateStudySchedule(config);

    if (sessions.empty()) {
        response["status"] = "error";
        response["message"] = "Failed to generate schedule";
        return response;
    }

    response["status"] = "success";
    response["message"] = "Schedule generated successfully";
    response["sessions_count"] = std::to_string(sessions.size());

    return response;
}

std::vector<std::map<std::string, std::string>> ScheduleController::handleGetSchedule(int userId, const std::string& startDate, const std::string& endDate) {
    std::vector<std::map<std::string, std::string>> schedule;

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT ss.*, t.name as topic_name, c.name as course_name, c.id as course_id "
        << "FROM study_sessions ss "
        << "JOIN topics t ON ss.topic_id = t.id "
        << "JOIN courses c ON t.course_id = c.id "
        << "WHERE ss.user_id = " << userId;

    if (!startDate.empty()) {
        oss << " AND date(ss.scheduled_start) >= '" << startDate << "'";
    }

    if (!endDate.empty()) {
        oss << " AND date(ss.scheduled_start) <= '" << endDate << "'";
    }

    oss << " ORDER BY ss.scheduled_start ASC";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            schedule.push_back(row);
        }
    }

    return schedule;
}

std::map<std::string, std::string> ScheduleController::handleUpdateSession(int userId, int sessionId, const std::map<std::string, std::string>& requestData) {
    std::map<std::string, std::string> response;

    DBManager& db = DBManager::getInstance();

    // Verify session belongs to user
    std::ostringstream oss;
    oss << "SELECT ss.topic_id, c.id as course_id FROM study_sessions ss "
        << "JOIN topics t ON ss.topic_id = t.id "
        << "JOIN courses c ON t.course_id = c.id "
        << "WHERE ss.id = " << sessionId << " AND ss.user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && results.empty()) {
        response["status"] = "error";
        response["message"] = "Session not found";
        return response;
    }

    int topicId = std::stoi(results[0].at("topic_id"));
    int courseId = std::stoi(results[0].at("course_id"));

    // Update session
    oss.str();
    int performanceRating = requestData.count("performance_rating") ?
        std::stoi(requestData.at("performance_rating")) : 0;

    oss << "UPDATE study_sessions SET is_completed = 1, performance_rating = " << performanceRating
        << " WHERE id = " << sessionId << " AND user_id = " << userId;

    if (!db.executeQuery(oss.str())) {
        response["status"] = "error";
        response["message"] = "Failed to update session";
        return response;
    }

    // Update progress
    double hoursStudied = requestData.count("hours_studied") ?
        std::stod(requestData.at("hours_studied")) : 1.0;

    double completion = (hoursStudied / 1.0) * 100.0; // Simplified calculation

    oss.str();
    oss << "INSERT INTO progress (user_id, session_id, course_id, topic_id, hours_studied, completion_percentage) "
        << "VALUES (" << userId << ", " << sessionId << ", " << courseId << ", " << topicId
        << ", " << hoursStudied << ", " << completion << ")";
    db.executeQuery(oss.str());

    // Adjust schedule based on performance
    if (performanceRating > 0) {
        m_recommendationEngine.adjustSchedule(userId, sessionId, performanceRating);
    }

    response["status"] = "success";
    response["message"] = "Session completed successfully";

    return response;
}

std::vector<std::string> ScheduleController::handleGetRecommendations(int userId) {
    return m_recommendationEngine.getRecommendations(userId);
}

} // namespace study_planner
