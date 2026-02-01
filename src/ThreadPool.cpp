#include "ThreadPool.h"
#include <iostream>

// Constructor - Create a specified number of worker threads
ThreadPool::ThreadPool(size_t threads) {
    std::cout << "Thread pool constructor called, creating " << threads << " worker threads" << std::endl;
    
    for(size_t i = 0; i < threads; ++i) {
        workers.emplace_back(
            [this] { this->workerThread(); }
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

// get the number of threads in the pool
size_t ThreadPool::getThreadCount() const{
    return workers.size();
}
    
// get the number of active threads
size_t ThreadPool::getActiveThreadCount() const {
    return active_threads;
}

// get the number of tasks to be processed in the queue
size_t ThreadPool::getTaskCount() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.size();
}

// get the number of waiting threads
size_t ThreadPool::getWaitingThreadCount() const {
    size_t totalThreads = getThreadCount();
    size_t activeCount = getActiveThreadCount();
    // waiting threads = total threads - active threads
    return totalThreads - activeCount;
}

// get the number of completed tasks
size_t ThreadPool::getCompletedTaskCount() const {
    return completed_tasks;
}

// Get the number of failed tasks
size_t ThreadPool::getFailedTaskCount() const {
    return failed_tasks; // Assuming failed_tasks is a member variable
}

// Working thread function - The core logic of the thread pool
void ThreadPool::workerThread() {
    while(true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
           // std::cout << "Worker thread " << std::this_thread::get_id() << " waiting for task or stop signal..." << std::endl;
            // Wait until there is a task or the thread pool stops
            condition.wait(lock, [this] {
                return this->stop || !this->tasks.empty();
            });
            
            // If the thread pool stops and the task queue is empty, exit the thread
            if(this->stop && this->tasks.empty()) {
                return;
            }
            
            // Get the task
            if(!this->tasks.empty()) {
                task = std::move(this->tasks.front());
                this->tasks.pop();
            }
        }
        
        // Execute the task (execute outside the lock to avoid blocking other threads)
         if(task) {
            ++active_threads; // Update active thread count
            try{
                task();
            } catch(...){
                // Handle exceptions thrown by tasks
            }
            // Update active and completed task counts
            --active_threads;
            ++completed_tasks;      
        }
    }
}