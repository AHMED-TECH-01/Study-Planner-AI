/**
 * @file RecommendationEngine.h
 * @brief Recommendation Engine - Adaptive scheduling based on performance
 *
 * This class implements the AI-powered adaptive scheduling that adjusts
 * study sessions based on student performance and deadlines.
 */

#ifndef RECOMMENDATION_ENGINE_H
#define RECOMMENDATION_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include "../models/StudySession.h"

namespace study_planner {

/**
 * @struct PerformanceData
 * @brief Performance data for a topic
 */
struct PerformanceData {
    int topicId;
    int courseId;
    double averageRating;
    int sessionsCompleted;
    int sessionsTotal;
    double masteryLevel; // 0-100%
};

/**
 * @class RecommendationEngine
 * @brief Provides adaptive scheduling recommendations
 *
 * Analyzes student performance and adjusts study schedules accordingly.
 * Uses performance ratings to determine topic mastery and suggests
 * additional review sessions when needed.
 */
class RecommendationEngine {
public:
    /**
     * @brief Default constructor
     */
    RecommendationEngine();

    /**
     * @brief Destructor
     */
    ~RecommendationEngine();

    /**
     * @brief Adjust schedule based on performance data
     * @param userId User ID
     * @param sessionId Completed session ID
     * @param performanceRating Performance rating (1-5)
     * @return true if adjustment was made
     */
    bool adjustSchedule(int userId, int sessionId, int performanceRating);

    /**
     * @brief Prioritize deadlines for schedule generation
     * @param userId User ID
     * @return Map of course IDs to priority scores
     */
    std::map<int, double> prioritizeDeadlines(int userId);

    /**
     * @brief Get recommendations for a user
     * @param userId User ID
     * @return Vector of recommendation strings
     */
    std::vector<std::string> getRecommendations(int userId);

    /**
     * @brief Generate review sessions for weak topics
     * @param userId User ID
     * @return Vector of new study sessions to add
     */
    std::vector<StudySession> generateReviewSessions(int userId);

    /**
     * @brief Analyze performance for a topic
     * @param topicId Topic ID
     * @return PerformanceData for the topic
     */
    PerformanceData analyzeTopicPerformance(int topicId);

private:
    /**
     * @brief Get average performance rating for a topic
     * @param topicId Topic ID
     * @return Average rating
     */
    double getAverageRating(int topicId);

    /**
     * @brief Check if a topic needs review
     * @param topicId Topic ID
     * @return true if review needed
     */
    bool needsReview(int topicId);

    /**
     * @brief Add a review session
     * @param userId User ID
     * @param topicId Topic ID
     * @return true if session added
     */
    bool addReviewSession(int userId, int topicId);

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

#endif // RECOMMENDATION_ENGINE_H
