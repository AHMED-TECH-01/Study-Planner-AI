/**
 * @file Course.cpp
 * @brief Implementation of Course Model
 */

#include "Course.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

namespace study_planner {

Course::Course() : m_id(0), m_userId(0), m_name(""), m_difficulty(3), m_examDate(""), m_createdAt("") {}

Course::Course(int id, int userId, const std::string& name, int difficulty, const std::string& examDate)
    : m_id(id), m_userId(userId), m_name(name), m_difficulty(difficulty), m_examDate(examDate), m_createdAt("") {}

Course::~Course() {}

// Getters
int Course::getId() const { return m_id; }
int Course::getUserId() const { return m_userId; }
std::string Course::getName() const { return m_name; }
int Course::getDifficulty() const { return m_difficulty; }
std::string Course::getExamDate() const { return m_examDate; }
std::string Course::getCreatedAt() const { return m_createdAt; }

// Setters
void Course::setId(int id) { m_id = id; }
void Course::setUserId(int userId) { m_userId = userId; }
void Course::setName(const std::string& name) { m_name = name; }
void Course::setDifficulty(int difficulty) { m_difficulty = difficulty; }
void Course::setExamDate(const std::string& examDate) { m_examDate = examDate; }
void Course::setCreatedAt(const std::string& createdAt) { m_createdAt = createdAt; }

double Course::calculatePriority() const {
    if (m_examDate.empty()) {
        return static_cast<double>(m_difficulty) * 10.0;
    }

    // Parse exam date (expected format: YYYY-MM-DD)
    try {
        int year = std::stoi(m_examDate.substr(0, 4));
        int month = std::stoi(m_examDate.substr(5, 2));
        int day = std::stoi(m_examDate.substr(8, 2));

        std::tm examTm = {};
        examTm.tm_year = year - 1900;
        examTm.tm_mon = month - 1;
        examTm.tm_mday = day;

        std::time_t examTime = std::mktime(&examTm);
        std::time_t now = std::time(nullptr);

        double daysUntilExam = std::difftime(examTime, now) / (24 * 60 * 60);

        if (daysUntilExam <= 0) {
            // Exam has passed, very high priority
            return 100.0;
        }

        // Priority formula: (difficulty * 10) / daysUntilExam
        // Higher difficulty and closer exam = higher priority
        double priority = (static_cast<double>(m_difficulty) * 10.0) / daysUntilExam;
        return std::min(priority, 100.0); // Cap at 100

    } catch (...) {
        return static_cast<double>(m_difficulty) * 10.0;
    }
}

std::map<std::string, std::string> Course::toMap() const {
    return {
        {"id", std::to_string(m_id)},
        {"user_id", std::to_string(m_userId)},
        {"name", m_name},
        {"difficulty", std::to_string(m_difficulty)},
        {"exam_date", m_examDate},
        {"created_at", m_createdAt},
        {"priority", std::to_string(calculatePriority())}
    };
}

Course Course::fromMap(const std::map<std::string, std::string>& data) {
    Course course;
    if (data.count("id")) course.setId(std::stoi(data.at("id")));
    if (data.count("user_id")) course.setUserId(std::stoi(data.at("user_id")));
    if (data.count("name")) course.setName(data.at("name"));
    if (data.count("difficulty")) course.setDifficulty(std::stoi(data.at("difficulty")));
    if (data.count("exam_date")) course.setExamDate(data.at("exam_date"));
    if (data.count("created_at")) course.setCreatedAt(data.at("created_at"));
    return course;
}

bool Course::isValidDifficulty(int difficulty) {
    return difficulty >= 1 && difficulty <= 5;
}

} // namespace study_planner
