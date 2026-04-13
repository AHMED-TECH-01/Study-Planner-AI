/**
 * @file Course.h
 * @brief Course Model - Represents an academic course
 *
 * This class encapsulates course data including name, difficulty level,
 * exam date, and associated topics.
 */

#ifndef COURSE_H
#define COURSE_H

#include <string>
#include <vector>
#include <map>

namespace study_planner {

/**
 * @class Course
 * @brief Represents an academic course
 *
 * Contains course information including difficulty level, exam deadline,
 * and methods for calculating priority based on time until exam.
 */
class Course {
public:
    /**
     * @brief Default constructor
     */
    Course();

    /**
     * @brief Parameterized constructor
     * @param id Course ID
     * @param userId Owner's user ID
     * @param name Course name
     * @param difficulty Difficulty level (1-5)
     * @param examDate Exam date string
     */
    Course(int id, int userId, const std::string& name, int difficulty, const std::string& examDate);

    /**
     * @brief Destructor
     */
    ~Course();

    // Getters
    int getId() const;
    int getUserId() const;
    std::string getName() const;
    int getDifficulty() const;
    std::string getExamDate() const;
    std::string getCreatedAt() const;

    // Setters
    void setId(int id);
    void setUserId(int userId);
    void setName(const std::string& name);
    void setDifficulty(int difficulty);
    void setExamDate(const std::string& examDate);
    void setCreatedAt(const std::string& createdAt);

    /**
     * @brief Calculate priority score based on difficulty and time until exam
     * @return Priority score (higher = more urgent)
     */
    double calculatePriority() const;

    /**
     * @brief Convert course to map for JSON serialization
     * @return Map containing course data
     */
    std::map<std::string, std::string> toMap() const;

    /**
     * @brief Create Course object from database result
     * @param data Map containing course data from database
     * @return Course object
     */
    static Course fromMap(const std::map<std::string, std::string>& data);

    /**
     * @brief Validate difficulty level
     * @param difficulty Value to validate
     * @return true if valid (1-5)
     */
    static bool isValidDifficulty(int difficulty);

private:
    int m_id;
    int m_userId;
    std::string m_name;
    int m_difficulty;
    std::string m_examDate;
    std::string m_createdAt;
};

} // namespace study_planner

#endif // COURSE_H
