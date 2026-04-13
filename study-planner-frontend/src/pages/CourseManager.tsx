import { useState, useEffect } from 'react';
import { Plus, Trash2, BookOpen, Calendar, ChevronDown, ChevronUp, X } from 'lucide-react';
import { coursesAPI } from '../services/api';

interface Topic {
  id: number;
  name: string;
  estimated_hours: number;
  is_completed: boolean;
}

interface Course {
  id: number;
  name: string;
  difficulty: number;
  exam_date: string;
  topics: Topic[];
}

export default function CourseManager() {
  const [courses, setCourses] = useState<Course[]>([]);
  const [loading, setLoading] = useState(true);
  const [showAddCourse, setShowAddCourse] = useState(false);
  const [showAddTopic, setShowAddTopic] = useState<number | null>(null);
  const [expandedCourses, setExpandedCourses] = useState<Set<number>>(new Set());

  // Add Course Form
  const [newCourseName, setNewCourseName] = useState('');
  const [newCourseDifficulty, setNewCourseDifficulty] = useState(3);
  const [newCourseExamDate, setNewCourseExamDate] = useState('');

  // Add Topic Form
  const [newTopicName, setNewTopicName] = useState('');
  const [newTopicHours, setNewTopicHours] = useState(2);

  useEffect(() => {
    loadCourses();
  }, []);

  const loadCourses = async () => {
    try {
      const response = await coursesAPI.getAll();
      if (response.data.courses) {
        // Load topics for each course
        const coursesWithTopics = await Promise.all(
          response.data.courses.map(async (course: any) => {
            try {
              const topicsRes = await coursesAPI.getTopics(course.id);
              return {
                ...course,
                topics: topicsRes.data.topics || []
              };
            } catch {
              return { ...course, topics: [] };
            }
          })
        );
        setCourses(coursesWithTopics);
      }
    } catch (error) {
      console.error('Failed to load courses:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleAddCourse = async (e: React.FormEvent) => {
    e.preventDefault();
    try {
      await coursesAPI.add(newCourseName, newCourseDifficulty, newCourseExamDate || undefined);
      setShowAddCourse(false);
      setNewCourseName('');
      setNewCourseDifficulty(3);
      setNewCourseExamDate('');
      loadCourses();
    } catch (error) {
      console.error('Failed to add course:', error);
    }
  };

  const handleRemoveCourse = async (courseId: number) => {
    if (!confirm('Are you sure you want to remove this course?')) return;
    try {
      await coursesAPI.remove(courseId);
      loadCourses();
    } catch (error) {
      console.error('Failed to remove course:', error);
    }
  };

  const handleAddTopic = async (courseId: number, e: React.FormEvent) => {
    e.preventDefault();
    try {
      await coursesAPI.addTopic(courseId, newTopicName, newTopicHours);
      setShowAddTopic(null);
      setNewTopicName('');
      setNewTopicHours(2);
      loadCourses();
    } catch (error) {
      console.error('Failed to add topic:', error);
    }
  };

  const toggleExpand = (courseId: number) => {
    const newExpanded = new Set(expandedCourses);
    if (newExpanded.has(courseId)) {
      newExpanded.delete(courseId);
    } else {
      newExpanded.add(courseId);
    }
    setExpandedCourses(newExpanded);
  };

  const getDifficultyLabel = (difficulty: number) => {
    switch (difficulty) {
      case 1: return { label: 'Easy', color: 'bg-green-100 text-green-700' };
      case 2: return { label: 'Medium', color: 'bg-yellow-100 text-yellow-700' };
      case 3: return { label: 'Hard', color: 'bg-orange-100 text-orange-700' };
      case 4: return { label: 'Very Hard', color: 'bg-red-100 text-red-700' };
      case 5: return { label: 'Expert', color: 'bg-purple-100 text-purple-700' };
      default: return { label: 'Unknown', color: 'bg-gray-100 text-gray-700' };
    }
  };

  const getDaysUntilExam = (examDate: string) => {
    if (!examDate) return null;
    const exam = new Date(examDate);
    const today = new Date();
    const diffTime = exam.getTime() - today.getTime();
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    return diffDays;
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
          <h1 className="text-2xl font-bold text-gray-900">Course Manager</h1>
          <p className="text-gray-500">Manage your courses and topics</p>
        </div>
        <button
          onClick={() => setShowAddCourse(true)}
          className="inline-flex items-center px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700 transition"
        >
          <Plus className="h-5 w-5 mr-2" />
          Add Course
        </button>
      </div>

      {/* Add Course Modal */}
      {showAddCourse && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
          <div className="bg-white rounded-xl shadow-xl max-w-md w-full p-6">
            <div className="flex justify-between items-center mb-4">
              <h2 className="text-xl font-semibold text-gray-900">Add New Course</h2>
              <button onClick={() => setShowAddCourse(false)} className="text-gray-400 hover:text-gray-600">
                <X className="h-6 w-6" />
              </button>
            </div>
            <form onSubmit={handleAddCourse} className="space-y-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">Course Name</label>
                <input
                  type="text"
                  value={newCourseName}
                  onChange={(e) => setNewCourseName(e.target.value)}
                  className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500 focus:border-transparent"
                  placeholder="e.g., Advanced Mathematics"
                  required
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">Difficulty Level</label>
                <div className="flex items-center space-x-4">
                  <input
                    type="range"
                    min="1"
                    max="5"
                    value={newCourseDifficulty}
                    onChange={(e) => setNewCourseDifficulty(parseInt(e.target.value))}
                    className="flex-1"
                  />
                  <span className={`px-3 py-1 rounded-full text-sm font-medium ${getDifficultyLabel(newCourseDifficulty).color}`}>
                    {getDifficultyLabel(newCourseDifficulty).label}
                  </span>
                </div>
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-1">Exam Date (Optional)</label>
                <input
                  type="date"
                  value={newCourseExamDate}
                  onChange={(e) => setNewCourseExamDate(e.target.value)}
                  className="w-full px-4 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500 focus:border-transparent"
                />
              </div>
              <div className="flex justify-end space-x-3 pt-4">
                <button
                  type="button"
                  onClick={() => setShowAddCourse(false)}
                  className="px-4 py-2 text-gray-600 hover:text-gray-800"
                >
                  Cancel
                </button>
                <button
                  type="submit"
                  className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700"
                >
                  Add Course
                </button>
              </div>
            </form>
          </div>
        </div>
      )}

      {/* Courses List */}
      {courses.length === 0 ? (
        <div className="bg-white rounded-xl shadow-sm border border-gray-100 p-12 text-center">
          <BookOpen className="h-16 w-16 text-gray-300 mx-auto mb-4" />
          <h3 className="text-lg font-medium text-gray-900 mb-2">No courses yet</h3>
          <p className="text-gray-500 mb-6">Start by adding your first course to create a study plan</p>
          <button
            onClick={() => setShowAddCourse(true)}
            className="inline-flex items-center px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700"
          >
            <Plus className="h-5 w-5 mr-2" />
            Add Your First Course
          </button>
        </div>
      ) : (
        <div className="space-y-4">
          {courses.map((course) => {
            const difficultyInfo = getDifficultyLabel(course.difficulty);
            const daysUntil = getDaysUntilExam(course.exam_date);
            const isExpanded = expandedCourses.has(course.id);

            return (
              <div key={course.id} className="bg-white rounded-xl shadow-sm border border-gray-100 overflow-hidden">
                <div className="p-6">
                  <div className="flex items-start justify-between">
                    <div className="flex items-start space-x-4">
                      <div className="p-3 bg-indigo-100 rounded-lg">
                        <BookOpen className="h-6 w-6 text-indigo-600" />
                      </div>
                      <div>
                        <h3 className="text-lg font-semibold text-gray-900">{course.name}</h3>
                        <div className="flex items-center space-x-3 mt-2">
                          <span className={`px-2 py-1 rounded-full text-xs font-medium ${difficultyInfo.color}`}>
                            {difficultyInfo.label}
                          </span>
                          {course.exam_date && (
                            <span className="flex items-center text-sm text-gray-500">
                              <Calendar className="h-4 w-4 mr-1" />
                              {new Date(course.exam_date).toLocaleDateString()}
                            </span>
                          )}
                          {daysUntil !== null && daysUntil > 0 && (
                            <span className={`px-2 py-1 rounded-full text-xs font-medium ${
                              daysUntil <= 7 ? 'bg-red-100 text-red-700' :
                              daysUntil <= 30 ? 'bg-yellow-100 text-yellow-700' :
                              'bg-green-100 text-green-700'
                            }`}>
                              {daysUntil} days until exam
                            </span>
                          )}
                        </div>
                      </div>
                    </div>
                    <div className="flex items-center space-x-2">
                      <button
                        onClick={() => toggleExpand(course.id)}
                        className="p-2 text-gray-400 hover:text-gray-600 hover:bg-gray-100 rounded-lg"
                      >
                        {isExpanded ? <ChevronUp className="h-5 w-5" /> : <ChevronDown className="h-5 w-5" />}
                      </button>
                      <button
                        onClick={() => handleRemoveCourse(course.id)}
                        className="p-2 text-gray-400 hover:text-red-600 hover:bg-red-50 rounded-lg"
                      >
                        <Trash2 className="h-5 w-5" />
                      </button>
                    </div>
                  </div>

                  {/* Topics Section */}
                  {isExpanded && (
                    <div className="mt-6 pt-6 border-t border-gray-100">
                      <div className="flex justify-between items-center mb-4">
                        <h4 className="font-medium text-gray-900">Topics ({course.topics.length})</h4>
                        <button
                          onClick={() => setShowAddTopic(course.id)}
                          className="inline-flex items-center px-3 py-1.5 text-sm bg-gray-100 text-gray-700 rounded-lg hover:bg-gray-200"
                        >
                          <Plus className="h-4 w-4 mr-1" />
                          Add Topic
                        </button>
                      </div>

                      {/* Add Topic Form */}
                      {showAddTopic === course.id && (
                        <form onSubmit={(e) => handleAddTopic(course.id, e)} className="mb-4 p-4 bg-gray-50 rounded-lg">
                          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
                            <div className="md:col-span-2">
                              <input
                                type="text"
                                value={newTopicName}
                                onChange={(e) => setNewTopicName(e.target.value)}
                                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500"
                                placeholder="Topic name"
                                required
                              />
                            </div>
                            <div className="flex space-x-2">
                              <input
                                type="number"
                                value={newTopicHours}
                                onChange={(e) => setNewTopicHours(parseInt(e.target.value))}
                                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-indigo-500"
                                min="1"
                                max="100"
                              />
                              <button
                                type="submit"
                                className="px-4 py-2 bg-indigo-600 text-white rounded-lg hover:bg-indigo-700"
                              >
                                Add
                              </button>
                              <button
                                type="button"
                                onClick={() => setShowAddTopic(null)}
                                className="px-3 py-2 text-gray-500 hover:text-gray-700"
                              >
                                Cancel
                              </button>
                            </div>
                          </div>
                        </form>
                      )}

                      {course.topics.length === 0 ? (
                        <p className="text-sm text-gray-500 text-center py-4">No topics added yet</p>
                      ) : (
                        <div className="space-y-2">
                          {course.topics.map((topic) => (
                            <div
                              key={topic.id}
                              className={`flex items-center justify-between p-3 rounded-lg ${
                                topic.is_completed ? 'bg-green-50' : 'bg-gray-50'
                              }`}
                            >
                              <div className="flex items-center space-x-3">
                                <div className={`w-2 h-2 rounded-full ${
                                  topic.is_completed ? 'bg-green-500' : 'bg-gray-400'
                                }`} />
                                <span className={topic.is_completed ? 'text-gray-500 line-through' : 'text-gray-900'}>
                                  {topic.name}
                                </span>
                              </div>
                              <span className="text-sm text-gray-500">{topic.estimated_hours} hours</span>
                            </div>
                          ))}
                        </div>
                      )}
                    </div>
                  )}
                </div>
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}
