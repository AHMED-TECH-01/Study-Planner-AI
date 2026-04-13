import { useState, useEffect } from 'react';
import { ChevronLeft, ChevronRight, Calendar, Clock, PlayCircle, CheckCircle, X, Settings, Sparkles } from 'lucide-react';
import { scheduleAPI, coursesAPI } from '../services/api';

interface StudySession {
  id: number;
  topic_id: number;
  topic_name: string;
  course_id: number;
  course_name: string;
  scheduled_start: string;
  duration_minutes: number;
  is_completed: boolean;
  performance_rating: number;
}

interface Course {
  id: number;
  name: string;
}

const DAYS = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
const MONTHS = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];

export default function StudyPlanner() {
  const [currentDate, setCurrentDate] = useState(new Date());
  const [sessions, setSessions] = useState<StudySession[]>([]);
  const [courses, setCourses] = useState<Course[]>([]);
  const [loading, setLoading] = useState(true);
  const [selectedSession, setSelectedSession] = useState<StudySession | null>(null);
  const [showCompleteModal, setShowCompleteModal] = useState(false);
  const [showGenerateModal, setShowGenerateModal] = useState(false);
  const [performanceRating, setPerformanceRating] = useState(3);

  // Generate Schedule Form
  const [dailyStudyHours, setDailyStudyHours] = useState(4);
  const [sessionDuration, setSessionDuration] = useState(60);
  const [daysAhead, setDaysAhead] = useState(14);

  useEffect(() => {
    loadData();
  }, [currentDate]);

  const loadData = async () => {
    try {
      // Load sessions for current month
      const startDate = new Date(currentDate.getFullYear(), currentDate.getMonth(), 1);
      const endDate = new Date(currentDate.getFullYear(), currentDate.getMonth() + 1, 0);

      const [scheduleRes, coursesRes] = await Promise.all([
        scheduleAPI.get(startDate.toISOString().split('T')[0], endDate.toISOString().split('T')[0]),
        coursesAPI.getAll()
      ]);

      setSessions(scheduleRes.data.sessions || []);
      setCourses(coursesRes.data.courses || []);
    } catch (error) {
      console.error('Failed to load schedule:', error);
    } finally {
      setLoading(false);
    }
  };

  const getDaysInMonth = (date: Date) => {
    const year = date.getFullYear();
    const month = date.getMonth();
    const firstDay = new Date(year, month, 1);
    const lastDay = new Date(year, month + 1, 0);
    const daysInMonth = lastDay.getDate();
    const startingDay = firstDay.getDay();

    const days: Array<{ date: number | null; isCurrentMonth: boolean }> = [];

    // Add empty cells for days before the first day of the month
    for (let i = 0; i < startingDay; i++) {
      days.push({ date: null, isCurrentMonth: false });
    }

    // Add days of the month
    for (let i = 1; i <= daysInMonth; i++) {
      days.push({ date: i, isCurrentMonth: true });
    }

    return days;
  };

  const getSessionsForDay = (day: number) => {
    const dateStr = `${currentDate.getFullYear()}-${String(currentDate.getMonth() + 1).padStart(2, '0')}-${String(day).padStart(2, '0')}`;
    return sessions.filter(s => s.scheduled_start && s.scheduled_start.startsWith(dateStr));
  };

  const handlePreviousMonth = () => {
    setCurrentDate(new Date(currentDate.getFullYear(), currentDate.getMonth() - 1, 1));
  };

  const handleNextMonth = () => {
    setCurrentDate(new Date(currentDate.getFullYear(), currentDate.getMonth() + 1, 1));
  };

  const handleCompleteSession = async () => {
    if (!selectedSession) return;
    try {
      await scheduleAPI.complete(selectedSession.id, performanceRating);
      setShowCompleteModal(false);
      setSelectedSession(null);
      setPerformanceRating(3);
      loadData();
    } catch (error) {
      console.error('Failed to complete session:', error);
    }
  };

  const handleGenerateSchedule = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await scheduleAPI.generate(dailyStudyHours, sessionDuration, daysAhead);
      setShowGenerateModal(false);
      loadData();
    } catch (error) {
      console.error('Failed to generate schedule:', error);
    }
  };

  const isToday = (day: number) => {
    const today = new Date();
    return (
      day === today.getDate() &&
      currentDate.getMonth() === today.getMonth() &&
      currentDate.getFullYear() === today.getFullYear()
    );
  };

  const days = getDaysInMonth(currentDate);

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="animate-spin rounded-full h-12 w-12 border-4 border-indigo-500 border-t-transparent"></div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex justify-between items-center">
        <div>
          <h1 className="text-2xl font-bold text-gray-900">Study Planner</h1>
          <p className="text-gray-500">Plan and manage your study sessions</p>
        </div>
        <button
          onClick={() => setShowGenerateModal(true)}
          className="inline-flex items-center px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700 transition"
        >
          <Sparkles className="h-5 w-5 mr-2" />
          Generate Schedule
        </button>
      </div>

      {/* Calendar */}
      <div className="bg-white rounded-xl shadow-sm border border-gray-100 overflow-hidden">
        {/* Calendar Header */}
        <div className="flex items-center justify-between p-4 border-b border-gray-100">
          <button
            onClick={handlePreviousMonth}
            className="p-2 hover:bg-gray-100 rounded-lg transition"
          >
            <ChevronLeft className="h-5 w-5 text-gray-600" />
          </button>
          <h2 className="text-lg font-semibold text-gray-900">
            {MONTHS[currentDate.getMonth()]} {currentDate.getFullYear()}
          </h2>
          <button
            onClick={handleNextMonth}
            className="p-2 hover:bg-gray-100 rounded-lg transition"
          >
            <ChevronRight className="h-5 w-5 text-gray-600" />
          </button>
        </div>

        {/* Calendar Grid */}
        <div className="p-4">
          {/* Day Headers */}
          <div className="grid grid-cols-7 gap-2 mb-2">
            {DAYS.map((day) => (
              <div key={day} className="text-center text-sm font-medium text-gray-500 py-2">
                {day}
              </div>
            ))}
          </div>

          {/* Calendar Days */}
          <div className="grid grid-cols-7 gap-2">
            {days.map((dayInfo, index) => {
              if (!dayInfo.isCurrentMonth) {
                return <div key={index} className="h-24 bg-gray-50 rounded-lg" />;
              }

              const daySessions = getSessionsForDay(dayInfo.date!);
              const todayClass = isToday(dayInfo.date!) ? 'ring-2 ring-indigo-500' : '';

              return (
                <div
                  key={index}
                  className={`h-24 bg-gray-50 rounded-lg p-2 ${todayClass}`}
                >
                  <div className={`text-sm font-medium mb-1 ${
                    isToday(dayInfo.date!) ? 'text-indigo-600' : 'text-gray-700'
                  }`}>
                    {dayInfo.date}
                  </div>
                  <div className="space-y-1 overflow-hidden">
                    {daySessions.slice(0, 2).map((session) => (
                      <button
                        key={session.id}
                        onClick={() => {
                          setSelectedSession(session);
                          if (!session.is_completed) {
                            setShowCompleteModal(true);
                          }
                        }}
                        className={`w-full text-left text-xs px-1.5 py-1 rounded truncate ${
                          session.is_completed
                            ? 'bg-green-100 text-green-700'
                            : 'bg-indigo-100 text-indigo-700 hover:bg-indigo-200'
                        }`}
                      >
                        {session.is_completed ? '✓ ' : ''}{session.topic_name}
                      </button>
                    ))}
                    {daySessions.length > 2 && (
                      <div className="text-xs text-gray-500 px-1.5">
                        +{daySessions.length - 2} more
                      </div>
                    )}
                  </div>
                </div>
              );
            })}
          </div>
        </div>
      </div>

      {/* Generate Schedule Modal */}
      {showGenerateModal && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
          <div className="bg-white rounded-xl shadow-xl max-w-md w-full p-6">
            <div className="flex justify-between items-center mb-6">
              <div className="flex items-center space-x-2">
                <Sparkles className="h-5 w-5 text-indigo-600" />
                <h2 className="text-xl font-semibold text-gray-900">Generate Study Schedule</h2>
              </div>
              <button onClick={() => setShowGenerateModal(false)} className="text-gray-400 hover:text-gray-600">
                <X className="h-6 w-6" />
              </button>
            </div>

            {courses.length === 0 ? (
              <div className="text-center py-6">
                <Calendar className="h-12 w-12 text-gray-300 mx-auto mb-3" />
                <p className="text-gray-500 mb-4">You need to add courses first before generating a schedule</p>
                <a href="/courses" className="text-indigo-600 hover:underline">Go to Course Manager</a>
              </div>
            ) : (
              <form onSubmit={handleGenerateSchedule} className="space-y-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">Daily Study Hours</label>
                  <div className="flex items-center space-x-4">
                    <input
                      type="range"
                      min="1"
                      max="8"
                      value={dailyStudyHours}
                      onChange={(e) => setDailyStudyHours(parseInt(e.target.value))}
                      className="flex-1"
                    />
                    <span className="w-16 text-center font-medium text-indigo-600">{dailyStudyHours} hrs</span>
                  </div>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">Session Duration (minutes)</label>
                  <select
                    value={sessionDuration}
                    onChange={(e) => setSessionDuration(parseInt(e.target.value))}
                    className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500"
                  >
                    <option value={30}>30 minutes</option>
                    <option value={45}>45 minutes</option>
                    <option value={60}>60 minutes</option>
                    <option value={90}>90 minutes</option>
                    <option value={120}>120 minutes</option>
                  </select>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">Days Ahead</label>
                  <select
                    value={daysAhead}
                    onChange={(e) => setDaysAhead(parseInt(e.target.value))}
                    className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500"
                  >
                    <option value={7}>1 week</option>
                    <option value={14}>2 weeks</option>
                    <option value={21}>3 weeks</option>
                    <option value={30}>1 month</option>
                  </select>
                </div>

                <div className="pt-4">
                  <button
                    type="submit"
                    className="w-full px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700 transition"
                  >
                    Generate Schedule
                  </button>
                </div>
              </form>
            )}
          </div>
        </div>
      )}

      {/* Complete Session Modal */}
      {showCompleteModal && selectedSession && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
          <div className="bg-white rounded-xl shadow-xl max-w-md w-full p-6">
            <div className="flex justify-between items-center mb-6">
              <div className="flex items-center space-x-2">
                <CheckCircle className="h-5 w-5 text-green-600" />
                <h2 className="text-xl font-semibold text-gray-900">Complete Session</h2>
              </div>
              <button onClick={() => setShowCompleteModal(false)} className="text-gray-400 hover:text-gray-600">
                <X className="h-6 w-6" />
              </button>
            </div>

            <div className="mb-6">
              <h3 className="font-medium text-gray-900">{selectedSession.topic_name}</h3>
              <p className="text-sm text-gray-500">{selectedSession.course_name}</p>
              <div className="flex items-center text-sm text-gray-500 mt-2">
                <Clock className="h-4 w-4 mr-1" />
                {selectedSession.duration_minutes} minutes
              </div>
            </div>

            <div className="mb-6">
              <label className="block text-sm font-medium text-gray-700 mb-2">How well did you focus?</label>
              <div className="flex justify-between">
                {[1, 2, 3, 4, 5].map((rating) => (
                  <button
                    key={rating}
                    onClick={() => setPerformanceRating(rating)}
                    className={`w-12 h-12 rounded-lg font-medium transition ${
                      performanceRating === rating
                        ? 'bg-indigo-600 text-white'
                        : 'bg-gray-100 text-gray-600 hover:bg-gray-200'
                    }`}
                  >
                    {rating}
                  </button>
                ))}
              </div>
              <div className="flex justify-between text-xs text-gray-500 mt-1">
                <span>Poor</span>
                <span>Excellent</span>
              </div>
            </div>

            <div className="flex space-x-3">
              <button
                onClick={() => setShowCompleteModal(false)}
                className="flex-1 px-4 py-2 border border-gray-300 text-gray-700 rounded-lg hover:bg-gray-50"
              >
                Cancel
              </button>
              <button
                onClick={handleCompleteSession}
                className="flex-1 px-4 py-2 bg-green-600 text-white rounded-lg hover:bg-green-700"
              >
                Complete
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Session List Below Calendar */}
      <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-6">
        <h3 className="text-lg font-semibold text-gray-900 mb-4">Upcoming Sessions</h3>
        {sessions.filter(s => !s.is_completed).length === 0 ? (
          <p className="text-gray-500 text-center py-4">No upcoming sessions</p>
        ) : (
          <div className="space-y-3">
            {sessions.filter(s => !s.is_completed).slice(0, 5).map((session) => (
              <div
                key={session.id}
                className="flex items-center justify-between p-4 bg-gray-50 rounded-lg"
              >
                <div className="flex items-center space-x-4">
                  <div className="p-2 bg-indigo-100 rounded-lg">
                    <PlayCircle className="h-5 w-5 text-indigo-600" />
                  </div>
                  <div>
                    <p className="font-medium text-gray-900">{session.topic_name}</p>
                    <p className="text-sm text-gray-500">{session.course_name}</p>
                  </div>
                </div>
                <div className="text-right">
                  <p className="text-sm font-medium text-gray-900">
                    {new Date(session.scheduled_start).toLocaleDateString()}
                  </p>
                  <p className="text-xs text-gray-500">
                    {new Date(session.scheduled_start).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                  </p>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
