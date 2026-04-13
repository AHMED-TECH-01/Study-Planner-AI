/**
 * @file db_manager.cpp
 * @brief Implementation of Database Manager
 */

#include "db_manager.h"
#include <iostream>
#include <chrono>

namespace study_planner {

// Initialize static instance
DBManager& DBManager::getInstance() {
    static DBManager instance;
    return instance;
}

DBManager::DBManager() : m_db(nullptr), m_initialized(false) {}

DBManager::~DBManager() {
    close();
}

bool DBManager::initialize(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_initialized) {
        return true;
    }

    m_dbPath = dbPath;

    // Open database connection
    int rc = sqlite3_open(dbPath.c_str(), &m_db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }

    // Enable foreign keys
    sqlite3_exec(m_db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    // Create tables
    const char* createTablesSQL = R"(
        -- Users table
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

        -- Courses table
        CREATE TABLE IF NOT EXISTS courses (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            difficulty INTEGER DEFAULT 3,
            exam_date TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        -- Topics table
        CREATE TABLE IF NOT EXISTS topics (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            course_id INTEGER NOT NULL,
            name TEXT NOT NULL,
            estimated_hours REAL DEFAULT 1.0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(course_id) REFERENCES courses(id) ON DELETE CASCADE
        );

        -- Study Sessions table
        CREATE TABLE IF NOT EXISTS study_sessions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            topic_id INTEGER NOT NULL,
            user_id INTEGER NOT NULL,
            scheduled_start TEXT NOT NULL,
            duration_minutes INTEGER DEFAULT 60,
            is_completed INTEGER DEFAULT 0,
            performance_rating INTEGER,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(topic_id) REFERENCES topics(id) ON DELETE CASCADE,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
        );

        -- Progress table
        CREATE TABLE IF NOT EXISTS progress (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL,
            session_id INTEGER,
            course_id INTEGER NOT NULL,
            topic_id INTEGER,
            hours_studied REAL DEFAULT 0,
            completion_percentage REAL DEFAULT 0,
            recorded_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE,
            FOREIGN KEY(session_id) REFERENCES study_sessions(id) ON DELETE SET NULL,
            FOREIGN KEY(course_id) REFERENCES courses(id) ON DELETE CASCADE,
            FOREIGN KEY(topic_id) REFERENCES topics(id) ON DELETE SET NULL
        );
    )";

    char* errMsg = nullptr;
    rc = sqlite3_exec(m_db, createTablesSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }

    m_initialized = true;
    std::cout << "Database initialized successfully at: " << dbPath << std::endl;
    return true;
}

bool DBManager::executeQuery(const std::string& query) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized || !m_db) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool DBManager::executeSelect(const std::string& query, std::vector<std::map<std::string, std::string>>& results) {
    std::lock_guard<std::mutex> lock(m_mutex);

    results.clear();

    if (!m_initialized || !m_db) {
        std::cerr << "Database not initialized" << std::endl;
        return false;
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(m_db) << std::endl;
        return false;
    }

    int columnCount = sqlite3_column_count(stmt);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::map<std::string, std::string> row;
        for (int i = 0; i < columnCount; i++) {
            const char* columnName = sqlite3_column_name(stmt, i);
            const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row[columnName] = columnValue ? columnValue : "";
        }
        results.push_back(row);
    }
    
    if (rc != SQLITE_DONE) {
        std::cerr << "Failed to fetch rows: " << sqlite3_errmsg(m_db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}

int DBManager::getLastInsertRowId() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return static_cast<int>(sqlite3_last_insert_rowid(m_db));
}

sqlite3* DBManager::getConnection() {
    return m_db;
}

void DBManager::close() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
        m_initialized = false;
    }
}

} // namespace study_planner
