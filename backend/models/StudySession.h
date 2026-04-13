/**
 * @file StudySession.h
 * @brief StudySession Model - Represents a scheduled study session
 *
 * This class encapsulates study session data including timing,
 * duration, completion status, and performance rating.
 */

#ifndef STUDY_SESSION_H
#define STUDY_SESSION_H

#include <string>
#include <map>

namespace study_planner {

/**
 * @class StudySession
 * @brief Represents a scheduled study session
 *
 * Contains information about study sessions including topic,
 * scheduled time, duration, completion status, and performance rating.
 */
class StudySession {
public:
    /**
     * @brief Default constructor
     */
    StudySession();

    /**
     * @brief Parameterized constructor
     * @param id Session ID
     * @param topicId Associated topic ID
     * @param userId User ID
     * @param scheduledStart Scheduled start time
     * @param durationMinutes Session duration in minutes
     */
    StudySession(int id, int topicId, int userId, const std::string& scheduledStart, int durationMinutes);

    /**
     * @brief Destructor
     */
    ~StudySession();

    // Getters
    int getId() const;
    int getTopicId() const;
    int getUserId() const;
    std::string getScheduledStart() const;
    int getDurationMinutes() const;
    bool isCompleted() const;
    int getPerformanceRating() const;
    std::string getCreatedAt() const;

    // Setters
    void setId(int id);
    void setTopicId(int topicId);
    void setUserId(int userId);
    void setScheduledStart(const std::string& scheduledStart);
    void setDurationMinutes(int minutes);
    void setCompleted(bool completed);
    void setPerformanceRating(int rating);
    void setCreatedAt(const std::string& createdAt);

    /**
     * @brief Convert session to map for JSON serialization
     * @return Map containing session data
     */
    std::map<std::string, std::string> toMap() const;

    /**
     * @brief Create StudySession object from database result
     * @param data Map containing session data from database
     * @return StudySession object
     */
    static StudySession fromMap(const std::map<std::string, std::string>& data);

    /**
     * @brief Validate performance rating
     * @param rating Rating value to validate
     * @return true if valid (1-5)
     */
    static bool isValidRating(int rating);

private:
    int m_id;
    int m_topicId;
    int m_userId;
    std::string m_scheduledStart;
    int m_durationMinutes;
    bool m_isCompleted;
    int m_performanceRating;
    std::string m_createdAt;
};

} // namespace study_planner

#endif // STUDY_SESSION_H
