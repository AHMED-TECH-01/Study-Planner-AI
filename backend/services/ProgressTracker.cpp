/**
 * @file ProgressTracker.cpp
 * @brief Implementation of Progress Tracker Service
 */

#include "ProgressTracker.h"
#include "../database/db_manager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace study_planner {

ProgressTracker::ProgressTracker() {}

ProgressTracker::~ProgressTracker() {}

bool ProgressTracker::updateProgress(int userId, int sessionId, int courseId, int topicId,
                                      double hoursStudied, double completion) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "INSERT INTO progress (user_id, session_id, course_id, topic_id, hours_studied, completion_percentage) "
        << "VALUES (" << userId << ", " << sessionId << ", " << courseId << ", " << topicId
        << ", " << hoursStudied << ", " << completion << ")";

    return db.executeQuery(oss.str());
}

double ProgressTracker::calculateCompletion(int userId, int courseId) {
    double completedHours = getCompletedHours(userId, courseId);
    double totalHours = getTotalEstimatedHours(courseId);

    if (totalHours <= 0) {
        return 0.0;
    }

    double percentage = (completedHours / totalHours) * 100.0;
    return std::min(percentage, 100.0); // Cap at 100%
}

ProgressStats ProgressTracker::getProgressStats(int userId) {
    ProgressStats stats;
    stats.totalHoursStudied = 0.0;
    stats.completionPercentage = 0.0;
    stats.sessionsCompleted = 0;
    stats.totalSessions = 0;

    DBManager& db = DBManager::getInstance();

    // Get total hours studied
    std::ostringstream oss;
    oss << "SELECT COALESCE(SUM(hours_studied), 0) as total FROM progress WHERE user_id = " << userId;
    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        stats.totalHoursStudied = std::stod(results[0].at("total"));
    }

    // Get completed sessions
    oss.str("");
    oss << "SELECT COUNT(*) as total FROM study_sessions WHERE user_id = " << userId << " AND is_completed = 1";
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        stats.sessionsCompleted = std::stoi(results[0].at("total"));
    }

    // Get total sessions
    oss.str("");
    oss << "SELECT COUNT(*) as total FROM study_sessions WHERE user_id = " << userId;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        stats.totalSessions = std::stoi(results[0].at("total"));
    }

    // Get course-specific progress
    oss.str("");
    oss << "SELECT c.name, c.id FROM courses c WHERE c.user_id = " << userId;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            int courseId = std::stoi(row.at("id"));
            stats.courseProgress[row.at("name")] = calculateCompletion(userId, courseId);
        }
    }

    // Calculate overall completion
    if (stats.totalSessions > 0) {
        stats.completionPercentage = (static_cast<double>(stats.sessionsCompleted) / stats.totalSessions) * 100.0;
    }

    return stats;
}

std::map<std::string, std::string> ProgressTracker::getCourseProgress(int userId, int courseId) {
    std::map<std::string, std::string> progress;
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT c.name, c.difficulty, c.exam_date, "
        << "(SELECT COALESCE(SUM(hours_studied), 0) FROM progress WHERE course_id = c.id) as hours_studied, "
        << "(SELECT COUNT(*) FROM study_sessions ss JOIN topics t ON ss.topic_id = t.id WHERE t.course_id = c.id AND ss.is_completed = 1) as completed_sessions, "
        << "(SELECT COUNT(*) FROM study_sessions ss JOIN topics t ON ss.topic_id = t.id WHERE t.course_id = c.id) as total_sessions "
        << "FROM courses c WHERE c.id = " << courseId << " AND c.user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        progress = results[0];

        // Calculate completion percentage
        int completed = std::stoi(progress["completed_sessions"]);
        int total = std::stoi(progress["total_sessions"]);
        double percentage = total > 0 ? (static_cast<double>(completed) / total) * 100.0 : 0.0;

        std::ostringstream perctoss;
        perctoss << std::fixed << std::setprecision(1) << percentage;
        progress["completion_percentage"] = perctoss.str();
    }

    return progress;
}

std::map<std::string, std::string> ProgressTracker::getDailyStats(int userId, const std::string& date) {
    std::map<std::string, std::string> stats;
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT "
        << "COUNT(*) as sessions_count, "
        << "COALESCE(SUM(duration_minutes), 0) as total_minutes, "
        << "COALESCE(AVG(performance_rating), 0) as avg_rating "
        << "FROM study_sessions "
        << "WHERE user_id = " << userId << " AND date(scheduled_start) = '" << date << "'";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        stats = results[0];
    }

    return stats;
}

double ProgressTracker::getCompletedHours(int userId, int courseId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT COALESCE(SUM(hours_studied), 0) as total FROM progress WHERE user_id = " << userId << " AND course_id = " << courseId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return std::stod(results[0].at("total"));
    }

    return 0.0;
}

double ProgressTracker::getTotalEstimatedHours(int courseId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT COALESCE(SUM(estimated_hours), 0) as total FROM topics WHERE course_id = " << courseId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return std::stod(results[0].at("total"));
    }

    return 0.0;
}

} // namespace study_planner
