/**
 * @file Topic.cpp
 * @brief Implementation of Topic Model
 */

#include "Topic.h"
#include <sstream>

namespace study_planner {

Topic::Topic() : m_id(0), m_courseId(0), m_name(""), m_estimatedHours(1.0), m_createdAt("") {}

Topic::Topic(int id, int courseId, const std::string& name, double estimatedHours)
    : m_id(id), m_courseId(courseId), m_name(name), m_estimatedHours(estimatedHours), m_createdAt("") {}

Topic::~Topic() {}

// Getters
int Topic::getId() const { return m_id; }
int Topic::getCourseId() const { return m_courseId; }
std::string Topic::getName() const { return m_name; }
double Topic::getEstimatedHours() const { return m_estimatedHours; }
std::string Topic::getCreatedAt() const { return m_createdAt; }

// Setters
void Topic::setId(int id) { m_id = id; }
void Topic::setCourseId(int courseId) { m_courseId = courseId; }
void Topic::setName(const std::string& name) { m_name = name; }
void Topic::setEstimatedHours(double hours) { m_estimatedHours = hours; }
void Topic::setCreatedAt(const std::string& createdAt) { m_createdAt = createdAt; }

std::map<std::string, std::string> Topic::toMap() const {
    std::ostringstream oss;
    oss << m_estimatedHours;
    return {
        {"id", std::to_string(m_id)},
        {"course_id", std::to_string(m_courseId)},
        {"name", m_name},
        {"estimated_hours", oss.str()},
        {"created_at", m_createdAt}
    };
}

Topic Topic::fromMap(const std::map<std::string, std::string>& data) {
    Topic topic;
    if (data.count("id")) topic.setId(std::stoi(data.at("id")));
    if (data.count("course_id")) topic.setCourseId(std::stoi(data.at("course_id")));
    if (data.count("name")) topic.setName(data.at("name"));
    if (data.count("estimated_hours")) topic.setEstimatedHours(std::stod(data.at("estimated_hours")));
    if (data.count("created_at")) topic.setCreatedAt(data.at("created_at"));
    return topic;
}

} // namespace study_planner
