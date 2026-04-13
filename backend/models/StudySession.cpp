/**
 * @file StudySession.cpp
 * @brief Implementation of StudySession Model
 */

#include "StudySession.h"

namespace study_planner {

StudySession::StudySession()
    : m_id(0), m_topicId(0), m_userId(0), m_scheduledStart(""),
      m_durationMinutes(60), m_isCompleted(false), m_performanceRating(0), m_createdAt("") {}

StudySession::StudySession(int id, int topicId, int userId, const std::string& scheduledStart, int durationMinutes)
    : m_id(id), m_topicId(topicId), m_userId(userId),
      m_scheduledStart(scheduledStart), m_durationMinutes(durationMinutes),
      m_isCompleted(false), m_performanceRating(0), m_createdAt("") {}

StudySession::~StudySession() {}

// Getters
int StudySession::getId() const { return m_id; }
int StudySession::getTopicId() const { return m_topicId; }
int StudySession::getUserId() const { return m_userId; }
std::string StudySession::getScheduledStart() const { return m_scheduledStart; }
int StudySession::getDurationMinutes() const { return m_durationMinutes; }
bool StudySession::isCompleted() const { return m_isCompleted; }
int StudySession::getPerformanceRating() const { return m_performanceRating; }
std::string StudySession::getCreatedAt() const { return m_createdAt; }

// Setters
void StudySession::setId(int id) { m_id = id; }
void StudySession::setTopicId(int topicId) { m_topicId = topicId; }
void StudySession::setUserId(int userId) { m_userId = userId; }
void StudySession::setScheduledStart(const std::string& scheduledStart) { m_scheduledStart = scheduledStart; }
void StudySession::setDurationMinutes(int minutes) { m_durationMinutes = minutes; }
void StudySession::setCompleted(bool completed) { m_isCompleted = completed; }
void StudySession::setPerformanceRating(int rating) { m_performanceRating = rating; }
void StudySession::setCreatedAt(const std::string& createdAt) { m_createdAt = createdAt; }

std::map<std::string, std::string> StudySession::toMap() const {
    return {
        {"id", std::to_string(m_id)},
        {"topic_id", std::to_string(m_topicId)},
        {"user_id", std::to_string(m_userId)},
        {"scheduled_start", m_scheduledStart},
        {"duration_minutes", std::to_string(m_durationMinutes)},
        {"is_completed", m_isCompleted ? "1" : "0"},
        {"performance_rating", std::to_string(m_performanceRating)},
        {"created_at", m_createdAt}
    };
}

StudySession StudySession::fromMap(const std::map<std::string, std::string>& data) {
    StudySession session;
    if (data.count("id")) session.setId(std::stoi(data.at("id")));
    if (data.count("topic_id")) session.setTopicId(std::stoi(data.at("topic_id")));
    if (data.count("user_id")) session.setUserId(std::stoi(data.at("user_id")));
    if (data.count("scheduled_start")) session.setScheduledStart(data.at("scheduled_start"));
    if (data.count("duration_minutes")) session.setDurationMinutes(std::stoi(data.at("duration_minutes")));
    if (data.count("is_completed")) session.setCompleted(data.at("is_completed") == "1");
    if (data.count("performance_rating")) session.setPerformanceRating(std::stoi(data.at("performance_rating")));
    if (data.count("created_at")) session.setCreatedAt(data.at("created_at"));
    return session;
}

bool StudySession::isValidRating(int rating) {
    return rating >= 1 && rating <= 5;
}

} // namespace study_planner
