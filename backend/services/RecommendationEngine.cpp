/**
 * @file RecommendationEngine.cpp
 * @brief Implementation of Recommendation Engine
 */

#include "RecommendationEngine.h"
#include "../database/db_manager.h"
#include "../models/Course.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace study_planner {

RecommendationEngine::RecommendationEngine() {}

RecommendationEngine::~RecommendationEngine() {}

bool RecommendationEngine::adjustSchedule(int userId, int sessionId, int performanceRating) {
    DBManager& db = DBManager::getInstance();

    // Update session with performance rating
    std::ostringstream oss;
    oss << "UPDATE study_sessions SET is_completed = 1, performance_rating = " << performanceRating
        << " WHERE id = " << sessionId << " AND user_id = " << userId;

    if (!db.executeQuery(oss.str())) {
        return false;
    }

    // Get topic ID for this session
    oss.str("");
    oss << "SELECT topic_id FROM study_sessions WHERE id = " << sessionId;
    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        int topicId = std::stoi(results[0].at("topic_id"));

        // If performance is low (< 3), add a review session
        if (performanceRating < 3) {
            return addReviewSession(userId, topicId);
        }
    }

    return true;
}

std::map<int, double> RecommendationEngine::prioritizeDeadlines(int userId) {
    std::map<int, double> priorities; // courseId -> priority

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT id, difficulty, exam_date FROM courses WHERE user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            int courseId = std::stoi(row.at("id"));
            int difficulty = std::stoi(row.at("difficulty"));
            std::string examDate = row.at("exam_date");

            Course course(0, 0, "", difficulty, examDate);
            priorities[courseId] = course.calculatePriority();
        }
    }

    return priorities;
}

std::vector<std::string> RecommendationEngine::getRecommendations(int userId) {
    std::vector<std::string> recommendations;

    DBManager& db = DBManager::getInstance();

    // Find topics with low performance
    std::ostringstream oss;
    oss << "SELECT t.id, t.name, c.name as course_name, "
        << "AVG(ss.performance_rating) as avg_rating, COUNT(ss.id) as session_count "
        << "FROM topics t "
        << "JOIN courses c ON t.course_id = c.id "
        << "LEFT JOIN study_sessions ss ON t.id = ss.topic_id AND ss.is_completed = 1 "
        << "WHERE c.user_id = " << userId << " "
        << "GROUP BY t.id "
        << "HAVING avg_rating < 3 OR avg_rating IS NULL";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            std::string topicName = row.at("name");
            std::string courseName = row.at("course_name");

            if (row.at("avg_rating").empty()) {
                std::ostringstream rec;
                rec << "Start studying '" << topicName << "' from " << courseName;
                recommendations.push_back(rec.str());
            } else {
                double avgRating = std::stod(row.at("avg_rating"));
                std::ostringstream rec;
                rec << "Review '" << topicName << "' from " << courseName
                    << " (avg rating: " << std::fixed << std::setprecision(1) << avgRating << ")";
                recommendations.push_back(rec.str());
            }
        }
    }

    // Add deadline-based recommendations
    oss.str("");
    oss << "SELECT name, exam_date, difficulty FROM courses "
        << "WHERE user_id = " << userId << " AND exam_date IS NOT NULL "
        << "ORDER BY exam_date ASC LIMIT 3";

    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            std::string courseName = row.at("name");
            std::string examDate = row.at("exam_date");
            std::ostringstream rec;
            rec << "Upcoming exam: " << courseName << " on " << examDate;
            recommendations.push_back(rec.str());
        }
    }

    return recommendations;
}

std::vector<StudySession> RecommendationEngine::generateReviewSessions(int userId) {
    std::vector<StudySession> reviewSessions;

    DBManager& db = DBManager::getInstance();

    // Find topics that need review (low average rating)
    std::ostringstream oss;
    oss << "SELECT t.id, t.course_id, t.name, AVG(ss.performance_rating) as avg_rating "
        << "FROM topics t "
        << "JOIN courses c ON t.course_id = c.id "
        << "LEFT JOIN study_sessions ss ON t.id = ss.topic_id AND ss.is_completed = 1 "
        << "WHERE c.user_id = " << userId << " "
        << "GROUP BY t.id "
        << "HAVING avg_rating < 3";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        std::string today = getTodayDate();

        int sessionCount = 0;
        for (const auto& row : results) {
            if (sessionCount >= 5) break; // Limit review sessions

            int topicId = std::stoi(row.at("id"));

            StudySession session;
            session.setTopicId(topicId);
            session.setUserId(userId);
            session.setScheduledStart(addDays(today, sessionCount + 1) + " 19:00:00");
            session.setDurationMinutes(45); // Shorter review sessions
            session.setCompleted(false);
            session.setPerformanceRating(0);

            // Save to database
            oss.str("");
            oss << "INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed, performance_rating) "
                << "VALUES (" << session.getTopicId() << ", " << session.getUserId()
                << ", '" << session.getScheduledStart() << "', " << session.getDurationMinutes()
                << ", 0, 0)";

            if (db.executeQuery(oss.str())) {
                session.setId(db.getLastInsertRowId());
                reviewSessions.push_back(session);
                sessionCount++;
            }
        }
    }

    return reviewSessions;
}

PerformanceData RecommendationEngine::analyzeTopicPerformance(int topicId) {
    PerformanceData data;
    data.topicId = topicId;
    data.averageRating = 0.0;
    data.sessionsCompleted = 0;
    data.sessionsTotal = 0;
    data.masteryLevel = 0.0;
    data.courseId = 0;

    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT course_id FROM topics WHERE id = " << topicId;
    std::vector<std::map<std::string, std::string>> results;

    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        data.courseId = std::stoi(results[0].at("course_id"));
    }

    oss.str("");
    oss << "SELECT "
        << "COUNT(*) as total, "
        << "COALESCE(AVG(performance_rating), 0) as avg_rating "
        << "FROM study_sessions "
        << "WHERE topic_id = " << topicId << " AND is_completed = 1";

    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        data.sessionsCompleted = std::stoi(results[0].at("total"));
        data.averageRating = std::stod(results[0].at("avg_rating"));

        // Calculate mastery level (rating * 20 for 0-100 scale)
        data.masteryLevel = data.averageRating * 20.0;
    }

    oss.str("");
    oss << "SELECT COUNT(*) as total FROM study_sessions WHERE topic_id = " << topicId;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        data.sessionsTotal = std::stoi(results[0].at("total"));
    }

    return data;
}

double RecommendationEngine::getAverageRating(int topicId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT COALESCE(AVG(performance_rating), 0) as avg_rating "
        << "FROM study_sessions WHERE topic_id = " << topicId << " AND is_completed = 1";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        return std::stod(results[0].at("avg_rating"));
    }

    return 0.0;
}

bool RecommendationEngine::needsReview(int topicId) {
    return getAverageRating(topicId) < 3.0;
}

bool RecommendationEngine::addReviewSession(int userId, int topicId) {
    DBManager& db = DBManager::getInstance();

    std::string today = getTodayDate();
    std::string reviewDate = addDays(today, 1); // Tomorrow

    std::ostringstream oss;
    oss << "INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed, performance_rating) "
        << "VALUES (" << topicId << ", " << userId << ", '" << reviewDate << " 20:00:00', 45, 0, 0)";

    return db.executeQuery(oss.str());
}

std::string RecommendationEngine::getTodayDate() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string RecommendationEngine::addDays(const std::string& date, int days) {
    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day + days;

    std::mktime(&tm);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

} // namespace study_planner
