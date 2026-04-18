
#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <sqlite3.h>
#include <string>
#include <vector>
#include <map>
#include <mutex>

namespace study_planner {

/**
 * @class DBManager
 * @brief Singleton database manager for SQLite operations
 *
 * Provides thread-safe database operations and manages the SQLite connection
 * for the entire application.
 */
class DBManager {
public:
    // Delete copy constructor and assignment operator
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    /**
     * @brief Get the singleton instance of DBManager
     * @return Reference to the DBManager instance
     */
    static DBManager& getInstance();

    /**
     * @brief Initialize the database connection
     * @param dbPath Path to the SQLite database file
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::string& dbPath);

    /**
     * @brief Execute a SQL query without results
     * @param query SQL query string
     * @return true if successful, false otherwise
     */
    bool executeQuery(const std::string& query);

    /**
     * @brief Execute a SQL query and get results
     * @param query SQL query string
     * @param results Vector of maps containing result rows
     * @return true if successful, false otherwise
     */
    bool executeSelect(const std::string& query, std::vector<std::map<std::string, std::string>>& results);

    /**
     * @brief Get the last inserted row ID
     * @return Last inserted row ID
     */
    int getLastInsertRowId();

    /**
     * @brief Get the database connection pointer
     * @return Pointer to SQLite connection
     */
    sqlite3* getConnection();

    /**
     * @brief Close the database connection
     */
    void close();

private:
    // Private constructor for singleton
    DBManager();
    // Private destructor
    ~DBManager();

    sqlite3* m_db;              // SQLite connection
    std::string m_dbPath;       // Database file path
    std::mutex m_mutex;         // Thread safety
    bool m_initialized;         // Initialization flag
};

} // namespace study_planner

#endif // DB_MANAGER_H
