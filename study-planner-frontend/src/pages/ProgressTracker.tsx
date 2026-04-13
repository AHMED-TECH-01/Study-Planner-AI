import { useState, useEffect } from 'react';
import { BarChart3, TrendingUp, Clock, Target, CheckCircle, Flame, Award } from 'lucide-react';
import { progressAPI, scheduleAPI } from '../services/api';

interface ProgressStats {
  total_sessions?: number;
  completed_sessions?: number;
  total_hours?: number;
  average_rating?: number;
  current_streak?: number;
  completion_rate?: number;
}

interface WeeklyData {
  day: string;
  sessions: number;
}

export default function ProgressTracker() {
  const [stats, setStats] = useState<ProgressStats>({});
  const [weeklyData, setWeeklyData] = useState<WeeklyData[]>([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadProgressData();
  }, []);

  const loadProgressData = async () => {
    try {
      const [progressRes, scheduleRes] = await Promise.all([
        progressAPI.get(),
        scheduleAPI.get()
      ]);

      // Process progress stats
      if (progressRes.data) {
        setStats({
          total_sessions: progressRes.data.total_sessions || 0,
          completed_sessions: progressRes.data.completed_sessions || 0,
          total_hours: progressRes.data.total_hours || 0,
          average_rating: progressRes.data.average_rating || 0,
          current_streak: progressRes.data.current_streak || 0,
          completion_rate: progressRes.data.completion_rate || 0
        });
      }

      // Generate weekly data from sessions
      const sessions = scheduleRes.data.sessions || [];
      const last7Days = getLast7Days();

      const weeklyStats: WeeklyData[] = last7Days.map((date) => {
        const daySessions = sessions.filter((s: any) =>
          s.scheduled_start && s.scheduled_start.startsWith(date)
        );
        const completed = daySessions.filter((s: any) => s.is_completed).length;
        return {
          day: new Date(date).toLocaleDateString('en-US', { weekday: 'short' }),
          sessions: completed
        };
      });

      setWeeklyData(weeklyStats);

    } catch (error) {
      console.error('Failed to load progress data:', error);
    } finally {
      setLoading(false);
    }
  };

  const getLast7Days = () => {
    const days: string[] = [];
    for (let i = 6; i >= 0; i--) {
      const date = new Date();
      date.setDate(date.getDate() - i);
      days.push(date.toISOString().split('T')[0]);
    }
    return days;
  };

  const getCompletionRate = () => {
    if (stats.total_sessions && stats.total_sessions > 0) {
      return Math.round((stats.completed_sessions || 0) / stats.total_sessions * 100);
    }
    return 0;
  };

  const getPerformanceLevel = () => {
    const rating = stats.average_rating || 0;
    if (rating >= 4.5) return { level: 'Excellent', color: 'text-green-600', bg: 'bg-green-100' };
    if (rating >= 3.5) return { level: 'Good', color: 'text-blue-600', bg: 'bg-blue-100' };
    if (rating >= 2.5) return { level: 'Average', color: 'text-yellow-600', bg: 'bg-yellow-100' };
    return { level: 'Needs Improvement', color: 'text-red-600', bg: 'bg-red-100' };
  };

  const getMaxSessions = () => {
    return Math.max(...weeklyData.map(d => d.sessions), 1);
  };

  if (loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="animate-spin rounded-full h-12 w-12 border-4 border-indigo-500 border-t-transparent"></div>
      </div>
    );
  }

  const performance = getPerformanceLevel();
  const maxSessions = getMaxSessions();

  return (
    <div className="space-y-6">
      {/* Header */}
      <div>
        <h1 className="text-2xl font-bold text-gray-900">Progress Tracker</h1>
        <p className="text-gray-500">Track your learning journey</p>
      </div>

      {/* Stats Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Completion Rate</p>
              <p className="text-3xl font-bold text-indigo-600 mt-1">{getCompletionRate()}%</p>
              <div className="mt-2 h-2 bg-gray-100 rounded-full overflow-hidden">
                <div
                  className="h-full bg-indigo-500 rounded-full transition-all"
                  style={{ width: `${getCompletionRate()}%` }}
                />
              </div>
            </div>
            <div className="p-3 bg-indigo-50 rounded-lg">
              <Target className="h-6 w-6 text-indigo-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Sessions Completed</p>
              <p className="text-3xl font-bold text-green-600 mt-1">{stats.completed_sessions || 0}</p>
              <p className="text-xs text-gray-500 mt-1">of {stats.total_sessions || 0} total</p>
            </div>
            <div className="p-3 bg-green-50 rounded-lg">
              <CheckCircle className="h-6 w-6 text-green-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Study Streak</p>
              <p className="text-3xl font-bold text-orange-600 mt-1">{stats.current_streak || 0}</p>
              <p className="text-xs text-gray-500 mt-1">days in a row</p>
            </div>
            <div className="p-3 bg-orange-50 rounded-lg">
              <Flame className="h-6 w-6 text-orange-600" />
            </div>
          </div>
        </div>

        <div className="bg-white rounded-xl shadow-sm p-6 border border-gray-100">
          <div className="flex items-center justify-between">
            <div>
              <p className="text-sm text-gray-500">Avg. Focus Rating</p>
              <p className="text-3xl font-bold text-purple-600 mt-1">{(stats.average_rating || 0).toFixed(1)}</p>
              <p className={`text-xs font-medium mt-1 ${performance.color}`}>{performance.level}</p>
            </div>
            <div className="p-3 bg-purple-50 rounded-lg">
              <Award className="h-6 w-6 text-purple-600" />
            </div>
          </div>
        </div>
      </div>

      {/* Charts Row */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Weekly Activity Chart */}
        <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-6">
          <div className="flex items-center justify-between mb-6">
            <h3 className="text-lg font-semibold text-gray-900">Weekly Activity</h3>
            <TrendingUp className="h-5 w-5 text-gray-400" />
          </div>
          <div className="h-48 flex items-end justify-between gap-2">
            {weeklyData.map((data, index) => (
              <div key={index} className="flex flex-col items-center flex-1">
                <div className="w-full flex flex-col items-center">
                  <span className="text-xs text-gray-500 mb-1">{data.sessions}</span>
                  <div
                    className="w-full max-w-12 bg-gradient-to-t from-indigo-500 to-purple-500 rounded-t-md transition-all"
                    style={{ height: `${Math.max((data.sessions / maxSessions) * 120, 8)}px` }}
                  />
                </div>
                <span className="text-xs text-gray-500 mt-2">{data.day}</span>
              </div>
            ))}
          </div>
        </div>

        {/* Performance Overview */}
        <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-6">
          <div className="flex items-center justify-between mb-6">
            <h3 className="text-lg font-semibold text-gray-900">Performance Overview</h3>
            <BarChart3 className="h-5 w-5 text-gray-400" />
          </div>

          <div className="space-y-5">
            {/* Total Hours */}
            <div>
              <div className="flex justify-between text-sm mb-2">
                <span className="text-gray-600">Total Study Hours</span>
                <span className="font-medium text-gray-900">{stats.total_hours || 0} hrs</span>
              </div>
              <div className="h-3 bg-gray-100 rounded-full overflow-hidden">
                <div
                  className="h-full bg-gradient-to-r from-indigo-500 to-purple-500 rounded-full"
                  style={{ width: `${Math.min((stats.total_hours || 0) / 100 * 100, 100)}%` }}
                />
              </div>
            </div>

            {/* Completion Rate */}
            <div>
              <div className="flex justify-between text-sm mb-2">
                <span className="text-gray-600">Session Completion</span>
                <span className="font-medium text-gray-900">{getCompletionRate()}%</span>
              </div>
              <div className="h-3 bg-gray-100 rounded-full overflow-hidden">
                <div
                  className="h-full bg-gradient-to-r from-green-400 to-emerald-500 rounded-full"
                  style={{ width: `${getCompletionRate()}%` }}
                />
              </div>
            </div>

            {/* Focus Rating */}
            <div>
              <div className="flex justify-between text-sm mb-2">
                <span className="text-gray-600">Average Focus</span>
                <span className="font-medium text-gray-900">{(stats.average_rating || 0).toFixed(1)} / 5.0</span>
              </div>
              <div className="h-3 bg-gray-100 rounded-full overflow-hidden">
                <div
                  className="h-full bg-gradient-to-r from-yellow-400 to-orange-500 rounded-full"
                  style={{ width: `${((stats.average_rating || 0) / 5) * 100}%` }}
                />
              </div>
            </div>

            {/* Streak */}
            <div>
              <div className="flex justify-between text-sm mb-2">
                <span className="text-gray-600">Current Streak</span>
                <span className="font-medium text-gray-900">{stats.current_streak || 0} days</span>
              </div>
              <div className="h-3 bg-gray-100 rounded-full overflow-hidden">
                <div
                  className="h-full bg-gradient-to-r from-orange-400 to-red-500 rounded-full"
                  style={{ width: `${Math.min((stats.current_streak || 0) * 10, 100)}%` }}
                />
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Achievements Section */}
      <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-6">
        <h3 className="text-lg font-semibold text-gray-900 mb-6">Your Achievements</h3>
        <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-6 gap-4">
          <AchievementCard
            icon={<CheckCircle className="h-6 w-6" />}
            title="First Session"
            description="Complete your first study session"
            unlocked={(stats.completed_sessions ?? 0) >= 1}
          />
          <AchievementCard
            icon={<Flame className="h-6 w-6" />}
            title="On Fire"
            description="3-day study streak"
            unlocked={(stats.current_streak ?? 0) >= 3}
          />
          <AchievementCard
            icon={<Target className="h-6 w-6" />}
            title="Goal Getter"
            description="Complete 10 sessions"
            unlocked={(stats.completed_sessions ?? 0) >= 10}
          />
          <AchievementCard
            icon={<Clock className="h-6 w-6" />}
            title="Time Invested"
            description="Study for 10 hours"
            unlocked={(stats.total_hours ?? 0) >= 10}
          />
          <AchievementCard
            icon={<Award className="h-6 w-6" />}
            title="High Performer"
            description="4.5+ average rating"
            unlocked={(stats.average_rating ?? 0) >= 4.5}
          />
          <AchievementCard
            icon={<TrendingUp className="h-6 w-6" />}
            title="Consistent"
            description="7-day study streak"
            unlocked={(stats.current_streak ?? 0) >= 7}
          />
        </div>
      </div>
    </div>
  );
}

function AchievementCard({ icon, title, description, unlocked }: {
  icon: React.ReactNode;
  title: string;
  description: string;
  unlocked: boolean;
}) {
  return (
    <div className={`p-4 rounded-xl border-2 text-center transition ${
      unlocked
        ? 'bg-gradient-to-br from-indigo-50 to-purple-50 border-indigo-200'
        : 'bg-gray-50 border-gray-200 opacity-50'
    }`}>
      <div className={`mx-auto w-12 h-12 rounded-full flex items-center justify-center mb-3 ${
        unlocked ? 'bg-indigo-100 text-indigo-600' : 'bg-gray-200 text-gray-400'
      }`}>
        {icon}
      </div>
      <h4 className={`font-medium text-sm ${unlocked ? 'text-gray-900' : 'text-gray-500'}`}>
        {title}
      </h4>
      <p className="text-xs text-gray-500 mt-1">{description}</p>
    </div>
  );
}
