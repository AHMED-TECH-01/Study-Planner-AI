/**
 * @file ScheduleController.h
 * @brief Schedule Controller - Handles scheduling endpoints
 *
 * This controller handles study schedule endpoints including
 * generating schedules and retrieving them.
 */

#ifndef SCHEDULE_CONTROLLER_H
#define SCHEDULE_CONTROLLER_H

#include <string>
#include <vector>
#include <map>
#include "../services/Scheduler.h"
#include "../services/RecommendationEngine.h"

namespace study_planner {

/**
 * @class ScheduleController
 * @brief Handles schedule-related HTTP endpoints
 */
class ScheduleController {
public:
    /**
     * @brief Default constructor
     */
    ScheduleController();

    /**
     * @brief Destructor
     */
    ~ScheduleController();

    /**
     * @brief Handle generate schedule
     * @param userId User ID
     * @param requestData Schedule config data
     * @return Response map with generated sessions
     */
    std::map<std::string, std::string> handleGenerateSchedule(int userId, const std::map<std::string, std::string>& requestData);

    /**
     * @brief Handle get schedule
     * @param userId User ID
     * @param startDate Start date filter
     * @param endDate End date filter
     * @return Vector of session data maps
     */
    std::vector<std::map<std::string, std::string>> handleGetSchedule(int userId, const std::string& startDate, const std::string& endDate);

    /**
     * @brief Handle update session (complete session)
     * @param userId User ID
     * @param sessionId Session ID
     * @param requestData Update data
     * @return Response map
     */
    std::map<std::string, std::string> handleUpdateSession(int userId, int sessionId, const std::map<std::string, std::string>& requestData);

    /**
     * @brief Handle get recommendations
     * @param userId User ID
     * @return Vector of recommendation strings
     */
    std::vector<std::string> handleGetRecommendations(int userId);

private:
    Scheduler m_scheduler;
    RecommendationEngine m_recommendationEngine;
};

} // namespace study_planner

#endif // SCHEDULE_CONTROLLER_H
