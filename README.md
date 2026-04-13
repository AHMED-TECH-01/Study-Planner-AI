# AI-Powered Study Planner

An AI-powered study planner with adaptive scheduling that helps students generate and adapt study schedules automatically based on courses, deadlines, and progress.

## Project Structure

```
/workspace/
├── backend/              # C++ backend with Crow framework
│   ├── main.cpp         # Entry point
│   ├── server.cpp       # Main server with REST API endpoints
│   ├── controllers/     # API controllers (Auth, Course, Schedule, Progress)
│   ├── models/         # OOP models (User, Course, Topic, StudySession)
│   ├── services/       # Business logic (Scheduler, ProgressTracker, RecommendationEngine)
│   ├── database/       # SQLite database manager
│   └── include/        # Crow framework headers
├── study-planner-frontend/  # React frontend
│   ├── src/
│   │   ├── pages/      # Dashboard, CourseManager, StudyPlanner, ProgressTracker
│   │   ├── components/ # Layout, reusable components
│   │   ├── services/   # API client
│   │   └── App.tsx     # Main app with routing
│   └── dist/           # Built production files
└── README.md
```

## Features

1. **User Authentication** - Register and login system
2. **Course Management** - Add courses with difficulty levels and exam dates
3. **Topic Management** - Add topics to courses with estimated hours
4. **AI-Powered Scheduling** - Automatically generate study schedules
5. **Adaptive Scheduling** - Adjusts study sessions based on performance
6. **Progress Tracking** - Track completion rates, streaks, and achievements
7. **Dashboard** - Daily study plan with recommendations

## Running the Backend

### Prerequisites
- C++ compiler with C++14 support
- SQLite3 development libraries
- Crow framework (included in include/)

### Compilation

```bash
cd /workspace/backend

# Compile with g++
g++ -std=c++14 -pthread -o server \
    main.cpp server.cpp \
    database/db_manager.cpp \
    controllers/*.cpp \
    models/*.cpp \
    services/*.cpp \
    -lsqlite3 -lpthread

# Or use the existing server.cpp directly
g++ -std=c++14 -pthread -o server server.cpp \
    database/db_manager.cpp \
    controllers/*.cpp \
    models/*.cpp \
    services/*.cpp \
    -lsqlite3 -lpthread
```

### Running
```bash
./server
# Server starts on http://0.0.0.0:18080
```

## Running the Frontend

### Development
```bash
cd /workspace/study-planner-frontend
npm install
npm run dev
# Frontend runs on http://localhost:5173
```

### Production Build
```bash
cd /workspace/study-planner-frontend
npm run build
# Output in dist/ directory
```

## API Endpoints

| Method | Endpoint | Description |
|--------|----------|-------------|
| POST | /register | Register new user |
| POST | /login | User login |
| GET | /courses | Get all courses |
| POST | /courses | Add new course |
| DELETE | /courses/:id | Remove course |
| GET | /courses/:id/topics | Get course topics |
| POST | /courses/:id/topics | Add topic to course |
| GET | /schedule | Get study schedule |
| POST | /schedule/generate | Generate study schedule |
| PUT | /schedule/:id | Complete session |
| GET | /recommendations | Get AI recommendations |
| GET | /progress | Get progress stats |
| GET | /progress/daily | Get daily stats |

## Configuration

The frontend expects the backend at `http://localhost:18080` by default.

To change this, set the environment variable:
```bash
VITE_API_URL=http://your-backend-url:18080
```

## Tech Stack

- **Frontend**: React 18, TypeScript, TailwindCSS, Vite
- **Backend**: C++, Crow framework, SQLite
- **Architecture**: Object-Oriented Programming (OOP)

## Example Test Data

After starting the server, register a user and add courses:

1. Register: POST /register with `{"username": "student", "password": "study123"}`
2. Add Course: POST /courses with `{"name": "Mathematics", "difficulty": 3, "exam_date": "2026-04-15"}`
3. Add Topic: POST /courses/1/topics with `{"name": "Calculus", "estimated_hours": 10}`
4. Generate Schedule: POST /schedule/generate with `{"daily_study_hours": 4, "session_duration": 60, "days_ahead": 14}`

## Live Demo

Frontend deployed at: https://ihx5fbab5ypz.space.minimax.io

Note: The backend needs to be running locally for full functionality.
