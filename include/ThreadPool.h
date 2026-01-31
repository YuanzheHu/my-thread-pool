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
    
    // Exercise 1: Status query method
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