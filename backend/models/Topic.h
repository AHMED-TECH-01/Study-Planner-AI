/**
 * @file Topic.h
 * @brief Topic Model - Represents a topic within a course
 *
 * This class encapsulates topic data including name and estimated study hours.
 */

#ifndef TOPIC_H
#define TOPIC_H

#include <string>
#include <map>

namespace study_planner {

/**
 * @class Topic
 * @brief Represents a topic within an academic course
 *
 * Contains information about study topics including name and
 * estimated hours needed to complete.
 */
class Topic {
public:
    /**
     * @brief Default constructor
     */
    Topic();

    /**
     * @brief Parameterized constructor
     * @param id Topic ID
     * @param courseId Parent course ID
     * @param name Topic name
     * @param estimatedHours Estimated study hours
     */
    Topic(int id, int courseId, const std::string& name, double estimatedHours);

    /**
     * @brief Destructor
     */
    ~Topic();

    // Getters
    int getId() const;
    int getCourseId() const;
    std::string getName() const;
    double getEstimatedHours() const;
    std::string getCreatedAt() const;

    // Setters
    void setId(int id);
    void setCourseId(int courseId);
    void setName(const std::string& name);
    void setEstimatedHours(double hours);
    void setCreatedAt(const std::string& createdAt);

    /**
     * @brief Convert topic to map for JSON serialization
     * @return Map containing topic data
     */
    std::map<std::string, std::string> toMap() const;

    /**
     * @brief Create Topic object from database result
     * @param data Map containing topic data from database
     * @return Topic object
     */
    static Topic fromMap(const std::map<std::string, std::string>& data);

private:
    int m_id;
    int m_courseId;
    std::string m_name;
    double m_estimatedHours;
    std::string m_createdAt;
};

} // namespace study_planner

#endif // TOPIC_H
