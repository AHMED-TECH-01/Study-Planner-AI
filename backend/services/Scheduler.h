/**
 * @file Scheduler.h
 * @brief Scheduler Service - Generates adaptive study schedules
 *
 * This class implements the core scheduling algorithm that automatically
 * generates study schedules based on courses, topics, deadlines, and priorities.
 * Uses weighted round-robin algorithm for fair distribution.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <string>
#include <vector>
#include <map>
#include "../models/StudySession.h"

namespace study_planner {

/**
 * @struct ScheduleConfig
 * @brief Configuration for schedule generation
 */
struct ScheduleConfig {
    int userId;
    int dailyStudyHours;       // Default: 4 hours
    int sessionDurationMinutes; // Default: 60 minutes
    int daysAhead;              // Generate schedule for next N days
};

/**
 * @class Scheduler
 * @brief Generates and manages study schedules
 *
 * Implements an adaptive scheduling algorithm that:
 * - Prioritizes courses based on difficulty and exam proximity
 * - Distributes study time fairly across topics
 * - Considers user's daily available study time
 */
class Scheduler {
public:
    /**
     * @brief Default constructor
     */
    Scheduler();

    /**
     * @brief Destructor
     */
    ~Scheduler();

    /**
     * @brief Generate a study schedule for the user
     * @param config Schedule configuration
     * @return Vector of generated study sessions
     */
    std::vector<StudySession> generateStudySchedule(const ScheduleConfig& config);

    /**
     * @brief Update an existing schedule
     * @param userId User ID
     * @param newSessions New sessions to add
     * @return true if successful
     */
    bool updateSchedule(int userId, const std::vector<StudySession>& newSessions);

    /**
     * @brief Get schedule for a specific date range
     * @param userId User ID
     * @param startDate Start date (YYYY-MM-DD)
     * @param endDate End date (YYYY-MM-DD)
     * @return Vector of study sessions
     */
    std::vector<StudySession> getSchedule(int userId, const std::string& startDate, const std::string& endDate);

    /**
     * @brief Clear all scheduled sessions for a user
     * @param userId User ID
     * @return true if successful
     */
    bool clearSchedule(int userId);

    /**
     * @brief Get existing sessions for a user
     * @param userId User ID
     * @return Vector of existing sessions
     */
    std::vector<StudySession> getExistingSessions(int userId);

private:
    /**
     * @brief Calculate priority weight for a course
     * @param courseId Course ID
     * @return Priority weight
     */
    double calculateCoursePriority(int courseId);

    /**
     * @brief Get all topics for a user's courses
     * @param userId User ID
     * @return Vector of topic data (id, course_id, name, estimated_hours)
     */
    std::vector<std::map<std::string, std::string>> getUserTopics(int userId);

    /**
     * @brief Generate time slots for a day
     * @param date Date string (YYYY-MM-DD)
     * @param startHour Start hour (24h format)
     * @param durationMinutes Session duration
     * @return ISO datetime string
     */
    std::string generateTimeSlot(const std::string& date, int startHour, int durationMinutes);

    /**
     * @brief Get today's date as string
     * @return Date string (YYYY-MM-DD)
     */
    std::string getTodayDate();

    /**
     * @brief Add days to a date
     * @param date Original date
     * @param days Number of days to add
     * @return New date string
     */
    std::string addDays(const std::string& date, int days);
};

} // namespace study_planner

#endif // SCHEDULER_H
