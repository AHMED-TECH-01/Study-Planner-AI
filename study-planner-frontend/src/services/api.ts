import axios from 'axios';

const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:18080';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Add auth token to requests
api.interceptors.request.use((config) => {
  const token = localStorage.getItem('token');
  if (token) {
    config.headers.Authorization = `Bearer ${token}`;
  }
  return config;
});

// Handle auth errors
api.interceptors.response.use(
  (response) => response,
  (error) => {
    if (error.response?.status === 401) {
      localStorage.removeItem('token');
      localStorage.removeItem('userId');
      window.location.href = '/login';
    }
    return Promise.reject(error);
  }
);

// Auth API
export const authAPI = {
  register: (username: string, password: string) =>
    api.post('/register', { username, password }),
  login: (username: string, password: string) =>
    api.post('/login', { username, password }),
};

// Courses API
export const coursesAPI = {
  getAll: () => api.get('/courses'),
  add: (name: string, difficulty: number, exam_date?: string) =>
    api.post('/courses', { name, difficulty, exam_date }),
  remove: (id: number) => api.delete(`/courses/${id}`),
  getTopics: (courseId: number) => api.get(`/courses/${courseId}/topics`),
  addTopic: (courseId: number, name: string, estimated_hours?: number) =>
    api.post(`/courses/${courseId}/topics`, { name, estimated_hours }),
};

// Schedule API
export const scheduleAPI = {
  get: (startDate?: string, endDate?: string) =>
    api.get('/schedule', { params: { start_date: startDate, end_date: endDate } }),
  generate: (dailyStudyHours?: number, sessionDuration?: number, daysAhead?: number) =>
    api.post('/schedule/generate', {
      daily_study_hours: dailyStudyHours,
      session_duration: sessionDuration,
      days_ahead: daysAhead,
    }),
  complete: (sessionId: number, performanceRating: number) =>
    api.put(`/schedule/${sessionId}`, { performance_rating: performanceRating }),
  getRecommendations: () => api.get('/recommendations'),
};

// Progress API
export const progressAPI = {
  get: () => api.get('/progress'),
  getDaily: (date: string) => api.get('/progress/daily', { params: { date } }),
};

export default api;
