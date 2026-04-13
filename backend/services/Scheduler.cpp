/**
 * @file Scheduler.cpp
 * @brief Implementation of Scheduler Service
 */

#include "Scheduler.h"
#include "../database/db_manager.h"
#include "../models/Course.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <chrono>

namespace study_planner {

Scheduler::Scheduler() {}

Scheduler::~Scheduler() {}

std::vector<StudySession> Scheduler::generateStudySchedule(const ScheduleConfig& config) {
    std::vector<StudySession> sessions;

    // Clear existing schedule first
    clearSchedule(config.userId);

    // Get all topics for the user
    auto topics = getUserTopics(config.userId);
    if (topics.empty()) {
        return sessions;
    }

    // Calculate priorities for each course
    std::map<int, double> coursePriorities; // courseId -> priority
    for (const auto& topic : topics) {
        int courseId = std::stoi(topic.at("course_id"));
        if (coursePriorities.find(courseId) == coursePriorities.end()) {
            coursePriorities[courseId] = calculateCoursePriority(courseId);
        }
    }

    // Calculate total weight
    double totalWeight = 0.0;
    for (const auto& cp : coursePriorities) {
        totalWeight += cp.second;
    }

    // Generate sessions for each day
    std::string currentDate = getTodayDate();
    int dailySlots = (config.dailyStudyHours * 60) / config.sessionDurationMinutes;

    // Study hours distribution (e.g., 9am-12pm, 2pm-5pm, 7pm-9pm)
    std::vector<int> preferredHours = {9, 10, 11, 14, 15, 16, 19, 20};

    for (int day = 0; day < config.daysAhead; ++day) {
        std::string date = addDays(currentDate, day);
        int slotIndex = 0;

        for (int slot = 0; slot < dailySlots && slotIndex < static_cast<int>(preferredHours.size()); ++slot) {
            // Weighted round-robin to select topic
            double randomWeight = ((double)rand() / RAND_MAX) * totalWeight;
            double cumulativeWeight = 0.0;
            int selectedCourseId = -1;

            for (const auto& cp : coursePriorities) {
                cumulativeWeight += cp.second;
                if (randomWeight <= cumulativeWeight) {
                    selectedCourseId = cp.first;
                    break;
                }
            }

            if (selectedCourseId == -1) {
                selectedCourseId = coursePriorities.begin()->first;
            }

            // Find a topic from this course
            for (const auto& topic : topics) {
                int topicCourseId = std::stoi(topic.at("course_id"));
                if (topicCourseId == selectedCourseId) {
                    StudySession session;
                    session.setTopicId(std::stoi(topic.at("id")));
                    session.setUserId(config.userId);
                    session.setScheduledStart(generateTimeSlot(date, preferredHours[slotIndex], config.sessionDurationMinutes));
                    session.setDurationMinutes(config.sessionDurationMinutes);
                    session.setCompleted(false);
                    session.setPerformanceRating(0);

                    // Save to database
                    DBManager& db = DBManager::getInstance();
                    std::ostringstream oss;
                    oss << "INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed, performance_rating) "
                        << "VALUES (" << session.getTopicId() << ", " << session.getUserId()
                        << ", '" << session.getScheduledStart() << "', " << session.getDurationMinutes()
                        << ", 0, 0)";

                    if (db.executeQuery(oss.str())) {
                        session.setId(db.getLastInsertRowId());
                        sessions.push_back(session);
                    }

                    slotIndex++;
                    break;
                }
            }
        }
    }

    return sessions;
}

bool Scheduler::updateSchedule(int userId, const std::vector<StudySession>& newSessions) {
    DBManager& db = DBManager::getInstance();

    for (const auto& session : newSessions) {
        std::ostringstream oss;
        oss << "INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed, performance_rating) "
            << "VALUES (" << session.getTopicId() << ", " << userId
            << ", '" << session.getScheduledStart() << "', " << session.getDurationMinutes()
            << ", " << (session.isCompleted() ? "1" : "0") << ", " << session.getPerformanceRating() << ")";

        if (!db.executeQuery(oss.str())) {
            return false;
        }
    }

    return true;
}

std::vector<StudySession> Scheduler::getSchedule(int userId, const std::string& startDate, const std::string& endDate) {
    std::vector<StudySession> sessions;
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT * FROM study_sessions WHERE user_id = " << userId
        << " AND date(scheduled_start) >= '" << startDate << "'"
        << " AND date(scheduled_start) <= '" << endDate << "'"
        << " ORDER BY scheduled_start ASC";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            sessions.push_back(StudySession::fromMap(row));
        }
    }

    return sessions;
}

bool Scheduler::clearSchedule(int userId) {
    DBManager& db = DBManager::getInstance();
    std::ostringstream oss;
    oss << "DELETE FROM study_sessions WHERE user_id = " << userId << " AND is_completed = 0";
    return db.executeQuery(oss.str());
}

std::vector<StudySession> Scheduler::getExistingSessions(int userId) {
    std::vector<StudySession> sessions;
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT * FROM study_sessions WHERE user_id = " << userId << " ORDER BY scheduled_start ASC";

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results)) {
        for (const auto& row : results) {
            sessions.push_back(StudySession::fromMap(row));
        }
    }

    return sessions;
}

double Scheduler::calculateCoursePriority(int courseId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT difficulty, exam_date FROM courses WHERE id = " << courseId;

    std::vector<std::map<std::string, std::string>> results;
    if (db.executeSelect(oss.str(), results) && !results.empty()) {
        int difficulty = std::stoi(results[0].at("difficulty"));
        std::string examDate = results[0].at("exam_date");

        // Calculate priority based on difficulty and time until exam
        Course course(0, 0, "", difficulty, examDate);
        return course.calculatePriority();
    }

    return 5.0; // Default priority
}

std::vector<std::map<std::string, std::string>> Scheduler::getUserTopics(int userId) {
    DBManager& db = DBManager::getInstance();

    std::ostringstream oss;
    oss << "SELECT t.id, t.course_id, t.name, t.estimated_hours "
        << "FROM topics t "
        << "JOIN courses c ON t.course_id = c.id "
        << "WHERE c.user_id = " << userId;

    std::vector<std::map<std::string, std::string>> results;
    db.executeSelect(oss.str(), results);
    return results;
}

std::string Scheduler::generateTimeSlot(const std::string& date, int startHour, int durationMinutes) {
    std::ostringstream oss;
    oss << date << " " << std::setfill('0') << std::setw(2) << startHour
        << ":00:00";
    return oss.str();
}

std::string Scheduler::getTodayDate() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

std::string Scheduler::addDays(const std::string& date, int days) {
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
