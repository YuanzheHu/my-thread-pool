#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

class ThreadPool {
public:
    // Constructor to create a specified number of worker threads
    ThreadPool(size_t threads);
    
    // Disable copy constructor and assignment operator
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    // Destructor
    ~ThreadPool();
    
    // Submit a task to the thread pool
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type>;

    // get the number of threads in the pool
    size_t getThreadCount() const;
    
    // get the number of active threads
    size_t getActiveThreadCount() const;

    // get the number of tasks to be processed in the queue
    size_t getTaskCount();

    // get the number of waiting threads
    size_t getWaitingThreadCount() const;

    // get the number of completed tasks
    size_t getCompletedTaskCount() const;

    // get the number of failed tasks
    size_t getFailedTaskCount() const;

    // Dynamically resize the thread pool
    void resize(size_t threads);

    // Pause the thread pool
    void pause();

    // Resume the thread pool
    void resume();

    // Wait for all tasks to complete
    void waitForCompletion();

    // Clear the task queue
    void clearTasks();
        
    // Status query method
    bool isStopped() const { return stop; }
    
private:
    // Worker thread function
    void workerThread();
    
    // Container for worker threads
    std::vector<std::thread> workers;
    
    // Task queue
    std::queue<std::function<void()>> tasks;
    
    // Synchronization mechanisms
    std::mutex queue_mutex;
    std::condition_variable condition;
    
    // Control for stopping the thread pool
    std::atomic<bool> stop{false};

    // Count of active threads
    std::atomic<size_t> active_threads{0};
    // Count of completed tasks
    std::atomic<size_t> completed_tasks{0};
    std::atomic<size_t> failed_tasks{0};
};

// Template function implementation
template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) 
    -> std::future<typename std::invoke_result<F, Args...>::type> {
    
    using return_type = typename std::invoke_result<F, Args...>::type;

    // Create task wrapper
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    
    std::future<return_type> result = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        // Thread pool has stopped, cannot add tasks
        if(stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        
        // Add task to the queue
        tasks.emplace([task]() { (*task)(); });
    }
    
    condition.notify_one();
    return result;
}

#endif // THREAD_POOL_H