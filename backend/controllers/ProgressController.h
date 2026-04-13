/**
 * @file ProgressController.h
 * @brief Progress Controller - Handles progress endpoints
 *
 * This controller handles study progress endpoints including
 * logging progress and retrieving statistics.
 */

#ifndef PROGRESS_CONTROLLER_H
#define PROGRESS_CONTROLLER_H

#include <string>
#include <vector>
#include <map>
#include "../services/ProgressTracker.h"

namespace study_planner {

/**
 * @class ProgressController
 * @brief Handles progress-related HTTP endpoints
 */
class ProgressController {
public:
    /**
     * @brief Default constructor
     */
    ProgressController();

    /**
     * @brief Destructor
     */
    ~ProgressController();

    /**
     * @brief Handle get progress stats
     * @param userId User ID
     * @return Progress statistics map
     */
    std::map<std::string, std::string> handleGetProgress(int userId);

    /**
     * @brief Handle log progress
     * @param userId User ID
     * @param requestData Progress data
     * @return Response map
     */
    std::map<std::string, std::string> handleLogProgress(int userId, const std::map<std::string, std::string>& requestData);

    /**
     * @brief Handle get course progress
     * @param userId User ID
     * @param courseId Course ID
     * @return Course progress map
     */
    std::map<std::string, std::string> handleGetCourseProgress(int userId, int courseId);

    /**
     * @brief Handle get daily stats
     * @param userId User ID
     * @param date Date string
     * @return Daily stats map
     */
    std::map<std::string, std::string> handleGetDailyStats(int userId, const std::string& date);

private:
    ProgressTracker m_progressTracker;
};

} // namespace study_planner

#endif // PROGRESS_CONTROLLER_H
