import { useState, useEffect } from 'react';
import { Link } from 'react-router-dom';
import { BookOpen, Calendar, TrendingUp, Clock, PlayCircle, Lightbulb, ChevronRight, Target, Zap, AlertCircle } from 'lucide-react';
import { coursesAPI, scheduleAPI, progressAPI } from '../services/api';

interface TodaySession {
  id: number;
  topic_name: string;
  course_name: string;
  scheduled_start: string;
  duration_minutes: number;
  is_completed: boolean;
}

interface Recommendation {
  type: string;
  message: string;
  priority: string;
}

interface ProgressStats {
  total_sessions?: number;
  completed_sessions?: number;
  total_courses?: number;
  total_topics?: number;
  completion_rate?: number;
}

export default function Dashboard() {
  const [todaySessions, setTodaySessions] = useState<TodaySession[]>([]);
  const [recommendations, setRecommendations] = useState<Recommendation[]>([]);
  const [stats, setStats] = useState<ProgressStats>({});
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadDashboardData();
  }, []);

  const loadDashboardData = async () => {
    try {
      const [scheduleRes, progressRes, coursesRes] = await Promise.all([
        scheduleAPI.get(),
        progressAPI.get(),
        coursesAPI.getAll()
      ]);

      // Process sessions for today
      const allSessions = scheduleRes.data.sessions || [];
      const today = new Date().toISOString().split('T')[0];
      const todaySessionsData = allSessions.filter((s: any) =>
        s.scheduled_start && s.scheduled_start.startsWith(today)
      );
      setTodaySessions(todaySessionsData);

      // Set progress stats
      if (progressRes.data) {
        setStats({
          total_sessions: progressRes.data.total_sessions || 0,
          completed_sessions: progressRes.data.completed_sessions || 0,
          completion_rate: progressRes.data.completion_rate || 0
        });
      }

      // Get recommendations
      try {
        const recRes = await scheduleAPI.getRecommendations();
        setRecommendations(recRes.data.recommendations || []);
      } catch {
        setRecommendations([]);
      }

    } catch (error) {
      console.error('Failed to load dashboard data:', error);
    } finally {
      setLoading(false);
    }
  };

  const getPriorityColor = (priority?: string) => {
    if (!priority) return 'bg-gray-100 text-gray-700';
    switch (priority.toLowerCase()) {
      case 'high': return 'bg-red-100 text-red-700';
      case 'medium': return 'bg-yellow-100 text-yellow-700';
      case 'low': return 'bg-green-100 text-green-700';
      default: return 'bg-gray-100 text-gray-700';
    }
  };

  const getCompletionRate = () => {
    if (stats.total_sessions && stats.total_sessions > 0) {
      return Math.round((stats.completed_sessions || 0) / stats.total_sessions * 100);
    }
    return 0;
  };

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
          <h1 className="text-2xl font-bold text-gray-900">Welcome back!</h1>
          <p className="text-gray-500">Here's your study overview for today</p>
        </div>
        <div className="text-right">
          <p className="text-sm text-gray-500">{new Date().toLocaleDateString('en-US', { weekday: 'long', month: 'long', day: 'numeric' })}</p>
        </div>
      </div>

      {/* Stats Cards */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Completion Rate</p>
              <p className="text-2xl font-bold text-indigo-600">{getCompletionRate()}%</p>
            </div>
            <div className="p-3 bg-indigo-50 rounded-lg">
              <TrendingUp className="h-6 w-6 text-indigo-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Sessions Completed</p>
              <p className="text-2xl font-bold text-green-600">{stats.completed_sessions || 0}</p>
            </div>
            <div className="p-3 bg-green-50 rounded-lg">
              <Target className="h-6 w-6 text-green-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Today's Sessions</p>
              <p className="text-2xl font-bold text-purple-600">{todaySessions.length}</p>
            </div>
            <div className="p-3 bg-purple-50 rounded-lg">
              <Clock className="h-6 w-6 text-purple-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Total Sessions</p>
              <p className="text-2xl font-bold text-orange-600">{stats.total_sessions || 0}</p>
            </div>
            <div className="p-3 bg-orange-50 rounded-lg">
              <BookOpen className="h-6 w-6 text-orange-600" />
            </div>
          </div>
        </div>
      </div>

      {/* Main Content Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Today's Study Plan */}
        <div className="lg:col-span-2 bg-white rounded-xl shadow-sm border border-gray-100">
          <div className="p-6 border-b border-gray-100">
            <div className="flex justify-between items-center">
              <h2 className="text-lg font-semibold text-gray-900">Today's Study Plan</h2>
              <Link to="/schedule" className="text-sm text-indigo-600 hover:text-indigo-700 flex items-center">
                View Calendar <ChevronRight className="h-4 w-4 ml-1" />
              </Link>
            </div>
          </div>
          <div className="p-6">
            {todaySessions.length === 0 ? (
              <div className="text-center py-8">
                <Calendar className="h-12 w-12 text-gray-300 mx-auto mb-3" />
                <p className="text-gray-500">No study sessions scheduled for today</p>
                <Link to="/schedule" className="text-indigo-600 hover:text-indigo-700 text-sm mt-2 inline-block">
                  Generate a study schedule
                </Link>
              </div>
            ) : (
              <div className="space-y-4">
                {todaySessions.map((session) => (
                  <div
                    key={session.id}
                    className={`p-4 rounded-lg border ${
                      session.is_completed
                        ? 'bg-green-50 border-green-200'
                        : 'bg-gray-50 border-gray-200'
                    }`}
                  >
                    <div className="flex items-start justify-between">
                      <div className="flex items-start space-x-3">
                        <div className={`p-2 rounded-lg ${
                          session.is_completed ? 'bg-green-100' : 'bg-indigo-100'
                        }`}>
                          <PlayCircle className={`h-5 w-5 ${
                            session.is_completed ? 'text-green-600' : 'text-indigo-600'
                          }`} />
                        </div>
                        <div>
                          <p className="font-medium text-gray-900">{session.topic_name}</p>
                          <p className="text-sm text-gray-500">{session.course_name}</p>
                          <p className="text-xs text-gray-400 mt-1">
                            {new Date(session.scheduled_start).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
                            {' - '}
                            {session.duration_minutes} min
                          </p>
                        </div>
                      </div>
                      {session.is_completed && (
                        <span className="px-2 py-1 bg-green-100 text-green-700 text-xs font-medium rounded-full">
                          Completed
                        </span>
                      )}
                    </div>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>

        {/* AI Recommendations */}
        <div className="bg-white rounded-xl shadow-sm border border-gray-100">
          <div className="p-6 border-b border-gray-100">
            <div className="flex items-center space-x-2">
              <Lightbulb className="h-5 w-5 text-amber-500" />
              <h2 className="text-lg font-semibold text-gray-900">AI Recommendations</h2>
            </div>
          </div>
          <div className="p-6">
            {recommendations.length === 0 ? (
              <div className="text-center py-8">
                <Zap className="h-12 w-12 text-gray-300 mx-auto mb-3" />
                <p className="text-gray-500">Add courses to get personalized recommendations</p>
                <Link to="/courses" className="text-indigo-600 hover:text-indigo-700 text-sm mt-2 inline-block">
                  Manage Courses
                </Link>
              </div>
            ) : (
              <div className="space-y-4">
                {recommendations.map((rec, index) => (
                  <div key={index} className="p-4 bg-gray-50 rounded-lg">
                    <div className="flex items-start space-x-3">
                      <div className={`p-1.5 rounded-full ${getPriorityColor(rec.priority)}`}>
                        <AlertCircle className="h-4 w-4" />
                      </div>
                      <div className="flex-1">
                        <p className="text-sm font-medium text-gray-900 capitalize">{rec.type}</p>
                        <p className="text-sm text-gray-600 mt-1">{rec.message}</p>
                      </div>
                    </div>
                  </div>
                ))}
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Quick Actions */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
        <Link
          to="/courses"
          className="bg-gradient-to-r from-indigo-500 to-purple-600 rounded-xl p-6 text-white hover:from-indigo-600 hover:to-purple-700 transition-all"
        >
          <BookOpen className="h-8 w-8 mb-3" />
          <h3 className="text-lg font-semibold">Manage Courses</h3>
          <p className="text-sm opacity-90 mt-1">Add or remove courses and topics</p>
        </Link>

        <Link
          to="/schedule"
          className="bg-gradient-to-r from-green-500 to-teal-600 rounded-xl p-6 text-white hover:from-green-600 hover:to-teal-700 transition-all"
        >
          <Calendar className="h-8 w-8 mb-3" />
          <h3 className="text-lg font-semibold">Study Planner</h3>
          <p className="text-sm opacity-90 mt-1">View and manage your study calendar</p>
        </Link>

        <Link
          to="/progress"
          className="bg-gradient-to-r from-orange-500 to-red-600 rounded-xl p-6 text-white hover:from-orange-600 hover:to-red-700 transition-all"
        >
          <TrendingUp className="h-8 w-8 mb-3" />
          <h3 className="text-lg font-semibold">Track Progress</h3>
          <p className="text-sm opacity-90 mt-1">View your learning statistics</p>
        </Link>
      </div>
    </div>
  );
}
