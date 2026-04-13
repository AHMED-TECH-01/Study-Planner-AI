/**
 * Study Planner Backend - Express.js Implementation
 * This implements the same OOP architecture as defined in the C++ backend
 */

const express = require('express');
const sqlite3 = require('sqlite3').verbose();
const cors = require('cors');
const bodyParser = require('body-parser');

const app = express();
const PORT = 18080;

app.use(cors());
app.use(bodyParser.json());

// Database setup
const db = new sqlite3.Database('./study_planner.db');

// Initialize database tables
db.serialize(() => {
    db.run(`CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password_hash TEXT NOT NULL,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS courses (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        difficulty INTEGER DEFAULT 3,
        exam_date TEXT,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE CASCADE
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS topics (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        course_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        estimated_hours REAL DEFAULT 1.0,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY(course_id) REFERENCES courses(id) ON DELETE CASCADE
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS study_sessions (
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
    )`);

    db.run(`CREATE TABLE IF NOT EXISTS progress (
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
    )`);
});

// Simple hash function (for demo purposes)
function hashPassword(password) {
    let hash = 0;
    for (let i = 0; i < password.length; i++) {
        hash = ((hash << 5) - hash) + password.charCodeAt(i);
        hash |= 0;
    }
    return hash.toString();
}

// Generate simple token
function generateToken(userId) {
    return `token_user_${userId}_${Date.now()}`;
}

// Extract user ID from token
function getUserIdFromRequest(req) {
    const authHeader = req.headers.authorization;
    if (!authHeader) return null;

    const token = authHeader.replace('Bearer ', '');
    const match = token.match(/token_user_(\d+)_/);
    return match ? parseInt(match[1]) : null;
}

// ============= MODELS (OOP) =============

class User {
    constructor(id, username, passwordHash) {
        this.id = id;
        this.username = username;
        this.passwordHash = passwordHash;
    }

    static fromMap(data) {
        return new User(data.id, data.username, data.password_hash);
    }
}

class Course {
    constructor(id, userId, name, difficulty, examDate) {
        this.id = id;
        this.userId = userId;
        this.name = name;
        this.difficulty = difficulty;
        this.examDate = examDate;
    }

    calculatePriority() {
        if (!this.examDate) return this.difficulty * 10;

        const examDate = new Date(this.examDate);
        const today = new Date();
        const daysUntil = Math.ceil((examDate - today) / (1000 * 60 * 60 * 24));

        if (daysUntil <= 0) return 100;
        return Math.min((this.difficulty * 10) / daysUntil, 100);
    }

    static fromMap(data) {
        return new Course(data.id, data.user_id, data.name, data.difficulty, data.exam_date);
    }
}

// ============= SERVICES (OOP) =============

class AuthService {
    registerUser(username, password) {
        return new Promise((resolve, reject) => {
            const hash = hashPassword(password);
            db.run(`INSERT INTO users (username, password_hash) VALUES (?, ?)`,
                [username, hash],
                function(err) {
                    if (err) resolve(null);
                    else resolve(new User(this.lastID, username, hash));
                }
            );
        });
    }

    loginUser(username, password) {
        return new Promise((resolve, reject) => {
            const hash = hashPassword(password);
            db.get(`SELECT * FROM users WHERE username = ? AND password_hash = ?`,
                [username, hash],
                (err, row) => {
                    if (err || !row) resolve(null);
                    else resolve(User.fromMap(row));
                }
            );
        });
    }

    getUserById(userId) {
        return new Promise((resolve, reject) => {
            db.get(`SELECT * FROM users WHERE id = ?`, [userId], (err, row) => {
                resolve(row ? User.fromMap(row) : null);
            });
        });
    }
}

class CourseService {
    getCourses(userId) {
        return new Promise((resolve, reject) => {
            db.all(`SELECT * FROM courses WHERE user_id = ? ORDER BY created_at DESC`,
                [userId],
                (err, rows) => resolve(rows || [])
            );
        });
    }

    addCourse(userId, name, difficulty, examDate) {
        return new Promise((resolve, reject) => {
            db.run(`INSERT INTO courses (user_id, name, difficulty, exam_date) VALUES (?, ?, ?, ?)`,
                [userId, name, difficulty, examDate || null],
                function(err) {
                    if (err) resolve(null);
                    else {
                        const courseId = this.lastID;
                        // Add default topics
                        const topics = [
                            ['Chapter 1 - Basics', 2.0],
                            ['Chapter 2 - Intermediate', 3.0],
                            ['Chapter 3 - Advanced', 4.0]
                        ];
                        topics.forEach(([topicName, hours]) => {
                            db.run(`INSERT INTO topics (course_id, name, estimated_hours) VALUES (?, ?, ?)`,
                                [courseId, topicName, hours]);
                        });
                        resolve({ id: courseId, name, difficulty, examDate });
                    }
                }
            );
        });
    }

    removeCourse(userId, courseId) {
        return new Promise((resolve, reject) => {
            db.run(`DELETE FROM courses WHERE id = ? AND user_id = ?`, [courseId, userId], function(err) {
                resolve(this.changes > 0);
            });
        });
    }

    getTopics(userId, courseId) {
        return new Promise((resolve, reject) => {
            db.all(`SELECT t.* FROM topics t
                    JOIN courses c ON t.course_id = c.id
                    WHERE c.id = ? AND c.user_id = ?`,
                [courseId, userId],
                (err, rows) => resolve(rows || [])
            );
        });
    }

    addTopic(userId, courseId, name, estimatedHours) {
        return new Promise((resolve, reject) => {
            db.run(`INSERT INTO topics (course_id, name, estimated_hours) VALUES (?, ?, ?)`,
                [courseId, name, estimatedHours || 1.0],
                function(err) {
                    resolve(err ? null : { id: this.lastID, name, estimatedHours });
                }
            );
        });
    }
}

class SchedulerService {
    constructor() {
        this.recommendationEngine = new RecommendationEngineService();
    }

    generateSchedule(userId, config) {
        return new Promise(async (resolve, reject) => {
            const { dailyStudyHours = 4, sessionDurationMinutes = 60, daysAhead = 7 } = config;

            // Clear existing schedule
            await new Promise(r => db.run(`DELETE FROM study_sessions WHERE user_id = ? AND is_completed = 0`, [userId], r));

            // Get topics
            const topics = await new Promise(r => {
                db.all(`SELECT t.id, t.course_id, c.difficulty, c.exam_date
                        FROM topics t JOIN courses c ON t.course_id = c.id
                        WHERE c.user_id = ?`, [userId], (err, rows) => r(rows || []));
            });

            if (topics.length === 0) {
                resolve({ sessionsCount: 0, message: 'No topics found' });
                return;
            }

            // Calculate course priorities
            const coursePriorities = {};
            topics.forEach(t => {
                if (!coursePriorities[t.course_id]) {
                    const course = new Course(t.course_id, userId, '', t.difficulty, t.exam_date);
                    coursePriorities[t.course_id] = course.calculatePriority();
                }
            });

            const totalWeight = Object.values(coursePriorities).reduce((a, b) => a + b, 0);
            const dailySlots = (dailyStudyHours * 60) / sessionDurationMinutes;
            const preferredHours = [9, 10, 11, 14, 15, 16, 19, 20];

            let sessionsCount = 0;
            const today = new Date();

            for (let day = 0; day < daysAhead; day++) {
                const date = new Date(today);
                date.setDate(date.getDate() + day);
                const dateStr = date.toISOString().split('T')[0];

                for (let slot = 0; slot < dailySlots && slot < preferredHours.length; slot++) {
                    // Weighted random selection
                    const randomWeight = Math.random() * totalWeight;
                    let cumulative = 0;
                    let selectedCourseId = null;

                    for (const [courseId, priority] of Object.entries(coursePriorities)) {
                        cumulative += priority;
                        if (randomWeight <= cumulative) {
                            selectedCourseId = parseInt(courseId);
                            break;
                        }
                    }

                    if (!selectedCourseId) selectedCourseId = parseInt(Object.keys(coursePriorities)[0]);

                    // Find topic from selected course
                    const topic = topics.find(t => t.course_id === selectedCourseId);
                    if (!topic) continue;

                    const startHour = preferredHours[slot];
                    const scheduledStart = `${dateStr} ${String(startHour).padStart(2, '0')}:00:00`;

                    await new Promise(r => {
                        db.run(`INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed)
                                VALUES (?, ?, ?, ?, 0)`,
                            [topic.id, userId, scheduledStart, sessionDurationMinutes],
                            r
                        );
                    });
                    sessionsCount++;
                }
            }

            resolve({ sessionsCount, message: 'Schedule generated successfully' });
        });
    }

    getSchedule(userId, startDate, endDate) {
        return new Promise((resolve, reject) => {
            let query = `SELECT ss.*, t.name as topic_name, c.name as course_name
                        FROM study_sessions ss
                        JOIN topics t ON ss.topic_id = t.id
                        JOIN courses c ON t.course_id = c.id
                        WHERE ss.user_id = ?`;

            const params = [userId];
            if (startDate) { query += ` AND date(ss.scheduled_start) >= ?`; params.push(startDate); }
            if (endDate) { query += ` AND date(ss.scheduled_start) <= ?`; params.push(endDate); }
            query += ` ORDER BY ss.scheduled_start ASC`;

            db.all(query, params, (err, rows) => resolve(rows || []));
        });
    }

    updateSession(userId, sessionId, performanceRating) {
        return new Promise(async (resolve, reject) => {
            // Get session info
            const session = await new Promise(r => {
                db.get(`SELECT ss.*, t.course_id FROM study_sessions ss
                        JOIN topics t ON ss.topic_id = t.id
                        WHERE ss.id = ? AND ss.user_id = ?`,
                    [sessionId, userId], (err, row) => r(row));
            });

            if (!session) {
                resolve({ success: false, message: 'Session not found' });
                return;
            }

            // Update session
            await new Promise(r => db.run(
                `UPDATE study_sessions SET is_completed = 1, performance_rating = ? WHERE id = ?`,
                [performanceRating, sessionId], r
            ));

            // Log progress
            await new Promise(r => db.run(
                `INSERT INTO progress (user_id, session_id, course_id, topic_id, hours_studied, completion_percentage)
                 VALUES (?, ?, ?, ?, ?, ?)`,
                [userId, sessionId, session.course_id, session.topic_id, session.duration_minutes / 60, 100], r
            ));

            // Adjust schedule if performance is low
            if (performanceRating < 3) {
                await this.recommendationEngine.adjustSchedule(userId, session.topic_id);
            }

            resolve({ success: true, message: 'Session completed' });
        });
    }

    getRecommendations(userId) {
        return this.recommendationEngine.getRecommendations(userId);
    }
}

class RecommendationEngineService {
    async adjustSchedule(userId, topicId) {
        const tomorrow = new Date();
        tomorrow.setDate(tomorrow.getDate() + 1);
        const dateStr = tomorrow.toISOString().split('T')[0];

        await new Promise(r => db.run(
            `INSERT INTO study_sessions (topic_id, user_id, scheduled_start, duration_minutes, is_completed)
             VALUES (?, ?, ?, 45, 0)`,
            [topicId, userId, `${dateStr} 20:00:00`], r
        ));
    }

    getRecommendations(userId) {
        return new Promise((resolve) => {
            const recommendations = [];

            // Get low performing topics
            db.all(`SELECT t.name, c.name as course_name, AVG(ss.performance_rating) as avg_rating
                    FROM topics t
                    JOIN courses c ON t.course_id = c.id
                    LEFT JOIN study_sessions ss ON t.id = ss.topic_id AND ss.is_completed = 1
                    WHERE c.user_id = ?
                    GROUP BY t.id
                    HAVING avg_rating < 3 OR avg_rating IS NULL
                    LIMIT 5`,
                [userId],
                (err, rows) => {
                    rows.forEach(row => {
                        if (!row.avg_rating) {
                            recommendations.push(`Start studying '${row.name}' from ${row.course_name}`);
                        } else {
                            recommendations.push(`Review '${row.name}' from ${row.course_name} (avg: ${parseFloat(row.avg_rating).toFixed(1)})`);
                        }
                    });

                    // Get upcoming exams
                    db.all(`SELECT name, exam_date FROM courses
                            WHERE user_id = ? AND exam_date IS NOT NULL
                            ORDER BY exam_date ASC LIMIT 3`,
                        [userId],
                        (err, exams) => {
                            exams.forEach(exam => {
                                recommendations.push(`Upcoming exam: ${exam.name} on ${exam.exam_date}`);
                            });
                            resolve(recommendations);
                        }
                    );
                }
            );
        });
    }
}

class ProgressService {
    getProgress(userId) {
        return new Promise((resolve) => {
            const stats = {
                totalHoursStudied: 0,
                completionPercentage: 0,
                sessionsCompleted: 0,
                totalSessions: 0,
                courseProgress: {}
            };

            db.get(`SELECT COALESCE(SUM(hours_studied), 0) as total FROM progress WHERE user_id = ?`,
                [userId],
                (err, row) => {
                    stats.totalHoursStudied = row.total || 0;

                    db.get(`SELECT COUNT(*) as total FROM study_sessions WHERE user_id = ? AND is_completed = 1`,
                        [userId],
                        (err, row) => {
                            stats.sessionsCompleted = row.total || 0;

                            db.get(`SELECT COUNT(*) as total FROM study_sessions WHERE user_id = ?`,
                                [userId],
                                (err, row) => {
                                    stats.totalSessions = row.total || 0;
                                    if (stats.totalSessions > 0) {
                                        stats.completionPercentage = (stats.sessionsCompleted / stats.totalSessions) * 100;
                                    }

                                    // Get course progress
                                    db.all(`SELECT c.id, c.name FROM courses c WHERE c.user_id = ?`,
                                        [userId],
                                        (err, courses) => {
                                            let completed = 0;
                                            courses.forEach(course => {
                                                stats.courseProgress[course.name] = 0;
                                            });
                                            resolve(stats);
                                        }
                                    );
                                }
                            );
                        }
                    );
                }
            );
        });
    }

    getDailyStats(userId, date) {
        return new Promise((resolve) => {
            db.get(`SELECT COUNT(*) as sessions, COALESCE(SUM(duration_minutes), 0) as minutes,
                           COALESCE(AVG(performance_rating), 0) as avg_rating
                    FROM study_sessions
                    WHERE user_id = ? AND date(scheduled_start) = ?`,
                [userId, date],
                (err, row) => resolve(row || { sessions: 0, minutes: 0, avg_rating: 0 })
            );
        });
    }
}

// ============= CONTROLLERS =============

const authService = new AuthService();
const courseService = new CourseService();
const schedulerService = new SchedulerService();
const progressService = new ProgressService();

// ============= ROUTES =============

// Health check
app.get('/health', (req, res) => res.json({ status: 'ok' }));

// Auth routes
app.post('/register', async (req, res) => {
    const { username, password } = req.body;
    if (!username || !password) {
        return res.json({ status: 'error', message: 'Username and password required' });
    }

    const user = await authService.registerUser(username, password);
    if (!user) {
        return res.json({ status: 'error', message: 'Registration failed - username may exist' });
    }

    res.json({
        status: 'success',
        message: 'User registered',
        user_id: user.id,
        username: user.username,
        token: generateToken(user.id)
    });
});

app.post('/login', async (req, res) => {
    const { username, password } = req.body;

    const user = await authService.loginUser(username, password);
    if (!user) {
        return res.json({ status: 'error', message: 'Invalid credentials' });
    }

    res.json({
        status: 'success',
        message: 'Login successful',
        user_id: user.id,
        username: user.username,
        token: generateToken(user.id)
    });
});

// Course routes
app.get('/courses', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    courseService.getCourses(userId).then(courses => {
        res.json({ status: 'success', courses });
    });
});

app.post('/courses', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { name, difficulty, exam_date } = req.body;
    if (!name || !difficulty) {
        return res.json({ status: 'error', message: 'Name and difficulty required' });
    }

    courseService.addCourse(userId, name, difficulty, exam_date).then(course => {
        if (!course) return res.json({ status: 'error', message: 'Failed to add course' });
        res.json({ status: 'success', message: 'Course added', course_id: course.id });
    });
});

app.delete('/courses/:id', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    courseService.removeCourse(userId, parseInt(req.params.id)).then(success => {
        res.json({ status: success ? 'success' : 'error', message: success ? 'Course removed' : 'Failed' });
    });
});

app.get('/courses/:id/topics', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    courseService.getTopics(userId, parseInt(req.params.id)).then(topics => {
        res.json({ status: 'success', topics });
    });
});

app.post('/courses/:id/topics', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { name, estimated_hours } = req.body;
    courseService.addTopic(userId, parseInt(req.params.id), name, estimated_hours).then(topic => {
        res.json({ status: topic ? 'success' : 'error', message: topic ? 'Topic added' : 'Failed' });
    });
});

// Schedule routes
app.get('/schedule', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { start_date, end_date } = req.query;
    schedulerService.getSchedule(userId, start_date, end_date).then(sessions => {
        res.json({ status: 'success', sessions });
    });
});

app.post('/schedule/generate', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { daily_study_hours, session_duration, days_ahead } = req.body;
    schedulerService.generateSchedule(userId, { daily_study_hours, session_duration, days_ahead }).then(result => {
        res.json({ status: 'success', ...result });
    });
});

app.put('/schedule/:id', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { performance_rating } = req.body;
    schedulerService.updateSession(userId, parseInt(req.params.id), performance_rating).then(result => {
        res.json({ status: result.success ? 'success' : 'error', message: result.message });
    });
});

app.get('/recommendations', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    schedulerService.getRecommendations(userId).then(recommendations => {
        res.json({ status: 'success', recommendations });
    });
});

// Progress routes
app.get('/progress', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    progressService.getProgress(userId).then(stats => {
        res.json(stats);
    });
});

app.get('/progress/daily', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    const { date } = req.query;
    progressService.getDailyStats(userId, date).then(stats => {
        res.json(stats);
    });
});

app.post('/progress', (req, res) => {
    const userId = getUserIdFromRequest(req);
    if (!userId) return res.status(401).json({ status: 'error', message: 'Unauthorized' });

    res.json({ status: 'success', message: 'Progress logged' });
});

// Start server
app.listen(PORT, () => {
    console.log(`Study Planner Server running on http://localhost:${PORT}`);
});
