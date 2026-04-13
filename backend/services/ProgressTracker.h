/**
 * @file ProgressTracker.h
 * @brief Progress Tracker Service - Tracks study progress
 *
 * This class tracks study progress across courses and topics,
 * calculates completion percentages, and provides statistics.
 */

#ifndef PROGRESS_TRACKER_H
#define PROGRESS_TRACKER_H

#include <string>
#include <vector>
#include <map>

namespace study_planner {

/**
 * @struct ProgressStats
 * @brief Statistics for study progress
 */
struct ProgressStats {
    double totalHoursStudied;
    double completionPercentage;
    int sessionsCompleted;
    int totalSessions;
    std::map<std::string, double> courseProgress; // course name -> progress
};

/**
 * @class ProgressTracker
 * @brief Tracks and calculates study progress
 *
 * Monitors study sessions, calculates completion rates,
 * and provides progress statistics for the dashboard.
 */
class ProgressTracker {
public:
    /**
     * @brief Default constructor
     */
    ProgressTracker();

    /**
     * @brief Destructor
     */
    ~ProgressTracker();

    /**
     * @brief Update progress after completing a study session
     * @param userId User ID
     * @param sessionId Session ID
     * @param courseId Course ID
     * @param topicId Topic ID
     * @param hoursStudied Hours studied
     * @param completion Completion percentage for this session
     * @return true if successful
     */
    bool updateProgress(int userId, int sessionId, int courseId, int topicId,
                        double hoursStudied, double completion);

    /**
     * @brief Calculate completion percentage for a course
     * @param userId User ID
     * @param courseId Course ID
     * @return Completion percentage (0-100)
     */
    double calculateCompletion(int userId, int courseId);

    /**
     * @brief Get overall progress statistics
     * @param userId User ID
     * @return ProgressStats structure
     */
    ProgressStats getProgressStats(int userId);

    /**
     * @brief Get progress for a specific course
     * @param userId User ID
     * @param courseId Course ID
     * @return Course progress data
     */
    std::map<std::string, std::string> getCourseProgress(int userId, int courseId);

    /**
     * @brief Get daily study statistics
     * @param userId User ID
     * @param date Date (YYYY-MM-DD)
     * @return Map of daily statistics
     */
    std::map<std::string, std::string> getDailyStats(int userId, const std::string& date);

private:
    /**
     * @brief Get completed hours for a course
     * @param userId User ID
     * @param courseId Course ID
     * @return Total hours completed
     */
    double getCompletedHours(int userId, int courseId);

    /**
     * @brief Get total estimated hours for a course
     * @param courseId Course ID
     * @return Total estimated hours
     */
    double getTotalEstimatedHours(int courseId);
};

} // namespace study_planner

#endif // PROGRESS_TRACKER_H
