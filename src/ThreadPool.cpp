#include "ThreadPool.h"
#include <iostream>

// Constructor - Create a specified number of worker threads
ThreadPool::ThreadPool(size_t threads) {
    std::cout << "Thread pool constructor called, creating " << threads << " worker threads" << std::endl;
    
    for(size_t i = 0; i < threads; ++i) {
        workers.emplace_back(
            [this, i] { this->workerThread(i); }
        );
    }
    
    std::cout << "All worker threads created successfully" << std::endl;
}

// Destructor - Gracefully shut down the thread pool
ThreadPool::~ThreadPool() {
    std::cout << "Thread pool is starting to shut down..." << std::endl;
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    
    condition.notify_all();
    
    for(std::thread &worker : workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
    
    std::cout << "Thread pool has been closed"  << std::endl;
}

// Get the number of threads in the pool
size_t ThreadPool::getThreadCount() const{
    return workers.size();
}
    
// Get the number of active threads
size_t ThreadPool::getActiveThreadCount() const {
    return active_threads;
}

// Get the number of tasks to be processed in the queue
size_t ThreadPool::getTaskCount() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size();
}

// Get the number of waiting threads
size_t ThreadPool::getWaitingThreadCount() const {
    size_t totalThreads = getThreadCount();
    size_t activeCount = getActiveThreadCount();
    // Waiting threads = total threads - active threads
    return totalThreads - activeCount;
}

// Get the number of completed tasks
size_t ThreadPool::getCompletedTaskCount() const {
    return completed_tasks;
}

// Get the number of failed tasks
size_t ThreadPool::getFailedTaskCount() const {
    return failed_tasks; // Assuming failed_tasks is a member variable
}

// Dynamically adjust the thread pool size
void ThreadPool::resize(size_t threads) {
    std::unique_lock<std::mutex> lock(queue_mutex);

    // If the thread pool has stopped, resizing is not allowed
    if (stop) {
        throw std::runtime_error("resize on stopped ThreadPool");
    }

    // Get the current number of threads
    size_t oldSize = workers.size();

    std::cout << "Adjusting thread pool size: " << oldSize << " -> " << threads << std::endl;

    // If the new thread count is greater than the current count, add new threads
    if (threads > oldSize) {
        workers.reserve(threads);
        for (size_t i = oldSize; i < threads; ++i) {
            workers.emplace_back([this, i] { this->workerThread(i); });
        }
        std::cout << "Added " << (threads - oldSize) << " worker threads" << std::endl;
    }
    // If the new thread count is less than the current count, we need to reduce threads
    else if (threads < oldSize) {
        // Clear any previously marked threads to stop
        threadsToStop.clear();

        // Add thread IDs to the set of threads to stop
        for (size_t i = threads; i < oldSize; ++i) {
            threadsToStop.insert(i);
        }

        // Unlock and notify
        lock.unlock();
        condition.notify_all();

        // Wait for threads to finish
        for (size_t i = threads; i < oldSize; ++i) {
            if (workers[i].joinable()) {
                workers[i].join();
            }
        }

        // Reacquire lock and resize the container
        lock.lock();
        workers.resize(threads);
        std::cout << "Removed " << (oldSize - threads) << " worker threads" << std::endl;
    }
}

// Pause the thread pool
void ThreadPool::pause() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    paused = true;
    std::cout << "Thread pool has been paused" << std::endl;
}

// Resume the thread pool
void ThreadPool::resume() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        paused = false;
        std::cout << "Thread pool has been resumed" << std::endl;
    }
    condition.notify_all();
}

// Wait for all tasks to complete
void ThreadPool::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    std::cout << "Waiting for all tasks to complete..." << std::endl;
    waitCondition.wait(lock, [this] {
        return (tasks.empty() && active_threads == 0) || stop;
    });
    std::cout << "All tasks have been completed" << std::endl;
}

// Clear the task queue
void ThreadPool::clearTasks() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    size_t taskCount = tasks.size();
    
    std::queue<std::function<void()>> emptyQueue;
    std::swap(tasks, emptyQueue);
    
    std::cout << "Cleared task queue: " << taskCount << " tasks were removed" << std::endl;
}

// Worker thread function - with thread ID parameter
void ThreadPool::workerThread(size_t id) {
    while(true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            // Wait until there is a task, the thread pool stops, or the thread needs to exit
            condition.wait(lock, [this, id] {
                return this->stop ||
                       (!this->paused && !this->tasks.empty()) ||
                       (this->threadsToStop.find(id) != this->threadsToStop.end());
            });
            
            // First, check if the thread pool has stopped
            if(this->stop) {
                return;
            }
            
            // Check if the current thread needs to terminate
            if(this->threadsToStop.find(id) != this->threadsToStop.end()) {
                this->threadsToStop.erase(id);
                return;
            }
            
            // Finally, check if there is a task to execute
            if(!this->paused && !this->tasks.empty()) {
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }
        }
        
        // Execute the task and handle exceptions
        if(task) {
            ++active_threads;  // Increment active thread count
            try {
                task();
                ++completed_tasks;
            } catch(const std::exception& e) {
                std::cerr << "Exception occurred in task: " << e.what() << std::endl;
                ++failed_tasks;
            } catch(...) {
                std::cerr << "Unknown exception occurred in task" << std::endl;
                ++failed_tasks;
            }
            --active_threads;   // Decrement active thread count
            waitCondition.notify_all();
        }
    }
}