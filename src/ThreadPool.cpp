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
    
    std::cout << "Thread pool has been closed" << std::endl;
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
            task();
        }
    }
}



// if(task) {
        //     try {
        //         task();
        //     } catch (const std::exception& e) {
        //         std::cout << "Task execution exception: " << e.what() << std::endl;
        //     } catch (...) {
        //         std::cout << "Task execution unknown exception" << std::endl;
        //     }
        // }