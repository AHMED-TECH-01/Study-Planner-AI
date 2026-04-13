cmake_minimum_required(VERSION 3.10)
project(StudyPlanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SQLite3
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Crow framework (header-only, assume it's in include/crow)
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    main.cpp
    server.cpp
    database/db_manager.cpp
    models/User.cpp
    models/Course.cpp
    models/Topic.cpp
    models/StudySession.cpp
    services/Scheduler.cpp
    services/ProgressTracker.cpp
    services/RecommendationEngine.cpp
    services/AuthService.cpp
    controllers/AuthController.cpp
    controllers/CourseController.cpp
    controllers/ScheduleController.cpp
    controllers/ProgressController.cpp
)

# Create executable
add_executable(study_planner_server ${SOURCES})

# Link libraries
target_link_libraries(study_planner_server SQLite3::SQLite3 pthread)

# Enable testing (optional)
enable_testing()
