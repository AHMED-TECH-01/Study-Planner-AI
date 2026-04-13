/**
 * @file CourseController.h
 * @brief Course Controller - Handles course endpoints
 *
 * This controller handles course management endpoints including
 * adding, removing, and listing courses.
 */

#ifndef COURSE_CONTROLLER_H
#define COURSE_CONTROLLER_H

#include <string>
#include <vector>
#include <map>
#include "../models/Course.h"
#include "../models/Topic.h"

namespace study_planner {

/**
 * @class CourseController
 * @brief Handles course-related HTTP endpoints
 */
class CourseController {
public:
    /**
     * @brief Default constructor
     */
    CourseController();

    /**
     * @brief Destructor
     */
    ~CourseController();

    /**
     * @brief Handle get all courses
     * @param userId User ID
     * @return Vector of course data maps
     */
    std::vector<std::map<std::string, std::string>> handleGetCourses(int userId);

    /**
     * @brief Handle add course
     * @param userId User ID
     * @param requestData Course data
     * @return Response map
     */
    std::map<std::string, std::string> handleAddCourse(int userId, const std::map<std::string, std::string>& requestData);

    /**
     * @brief Handle remove course
     * @param userId User ID
     * @param courseId Course ID
     * @return Response map
     */
    std::map<std::string, std::string> handleRemoveCourse(int userId, int courseId);

    /**
     * @brief Handle get course topics
     * @param userId User ID
     * @param courseId Course ID
     * @return Vector of topic data maps
     */
    std::vector<std::map<std::string, std::string>> handleGetTopics(int userId, int courseId);

    /**
     * @brief Handle add topic
     * @param userId User ID
     * @param courseId Course ID
     * @param requestData Topic data
     * @return Response map
     */
    std::map<std::string, std::string> handleAddTopic(int userId, int courseId, const std::map<std::string, std::string>& requestData);

private:
    /**
     * @brief Validate course data
     * @param data Course data
     * @return true if valid
     */
    bool validateCourseData(const std::map<std::string, std::string>& data);
};

} // namespace study_planner

#endif // COURSE_CONTROLLER_H
