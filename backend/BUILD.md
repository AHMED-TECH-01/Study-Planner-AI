# Build and Run Instructions

## Option 1: Node.js Backend (Recommended)

```bash
cd backend
npm install
npm start
```

The server will start on http://localhost:18080

## Option 2: C++ Backend (Original Design)

The C++ backend is designed with OOP principles and uses the Crow framework.
Due to the complexity of setting up the Crow framework, we recommend using
the Node.js implementation which follows the same architecture.

### To compile the C++ backend (requires Crow and SQLite3):

```bash
cd backend
mkdir build && cd build
cmake ..
make
./study_planner_server
```

## API Endpoints

- POST /register - Register new user
- POST /login - Login user
- GET /courses - Get all courses
- POST /courses - Add course
- DELETE /courses/:id - Remove course
- GET /courses/:id/topics - Get course topics
- POST /courses/:id/topics - Add topic
- GET /schedule - Get study schedule
- POST /schedule/generate - Generate schedule
- PUT /schedule/:id - Update session (complete)
- GET /recommendations - Get recommendations
- GET /progress - Get progress stats
- GET /progress/daily - Get daily stats

## Test Data

After starting the server, use these credentials:
- Username: student1
- Password: password123
