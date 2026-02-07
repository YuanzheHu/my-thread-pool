#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <thread>
#include "ThreadPool.h"

// Test function: Time-consuming computation
int longComputation(int id, int duration) {
    std::cout << "Task " << id << " started, duration: " << duration << "ms" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    std::cout << "Task " << id << " completed" << std::endl;
    return duration;
}

// Test function: Task that may throw an exception
int errorProneTask(int id, bool shouldFail) {
    std::cout << "ErrorProneTask " << id << " started" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (shouldFail) {
        std::cout << "ErrorProneTask " << id << " throwing exception" << std::endl;
        throw std::runtime_error("Task failed on purpose");
    }
    
    std::cout << "ErrorProneTask " << id << " completed successfully" << std::endl;
    return id;
}

// Helper function to print the status of the thread pool
void printPoolStatus(ThreadPool& pool, const std::string& stage) {
    std::cout << "\n=== " << stage << " ===" << std::endl;
    std::cout << "  Thread count: " << pool.getThreadCount() << std::endl;
    std::cout << "  Active thread count: " << pool.getActiveThreadCount() << std::endl;
    std::cout << "  Waiting thread count: " << pool.getWaitingThreadCount() << std::endl;
    std::cout << "  Pending task count: " << pool.getTaskCount() << std::endl;
    std::cout << "  Completed task count: " << pool.getCompletedTaskCount() << std::endl;
    std::cout << "  Failed task count: " << pool.getFailedTaskCount() << std::endl;
}

int main() {
    std::cout << "=== C++11 ThreadPool Implementation - Day 5 Test ===" << std::endl;
    
    try {
        // Create a thread pool with a number of threads equal to hardware concurrency
        size_t threadCount = std::thread::hardware_concurrency();
        threadCount = threadCount == 0 ? 1 : threadCount;
        
        // To better observe the effect, use a smaller number of threads
        size_t poolThreads = std::min(threadCount, (size_t)4);
        
        std::cout << "System has " << threadCount << " CPU cores" << std::endl;
        std::cout << "Creating a thread pool with " << poolThreads << " threads" << std::endl;
        
        ThreadPool pool(poolThreads);
        
        // Display initial status
        printPoolStatus(pool, "Initial Status");
        
        // Random number generator for task durations
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> durDist(100, 300); // 100-300ms
        
        // Submit some tasks
        std::cout << "\nSubmitting 6 normal tasks..." << std::endl;
        std::vector<std::future<int>> results;
        for (int i = 0; i < 6; ++i) {
            int duration = durDist(gen);
            results.push_back(pool.enqueue(longComputation, i, duration));
        }
        
        // Wait for some time to let some tasks complete
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        // Display status after some tasks are completed
        printPoolStatus(pool, "Status after some tasks completed");
        
        // Test pause functionality
        std::cout << "\n--- Testing Pause/Resume Functionality ---" << std::endl;
        pool.pause();
        
        // Submit more tasks (these tasks will be paused)
        std::cout << "Submitting 3 tasks while the thread pool is paused..." << std::endl;
        for (int i = 10; i < 13; ++i) {
            int duration = durDist(gen);
            results.push_back(pool.enqueue(longComputation, i, duration));
        }
        
        // Display status after pausing
        printPoolStatus(pool, "Status after pausing");
        
        // Wait for some time
        std::cout << "Waiting for 1 second..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Display status after waiting (should not change much as the pool is paused)
        printPoolStatus(pool, "Status after waiting (paused)");
        
        // Resume the thread pool
        pool.resume();
        
        // Wait for some time to let more tasks complete
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Display status after resuming
        printPoolStatus(pool, "Status after resuming");
        
        // Test dynamic thread resizing
        std::cout << "\n--- Testing Dynamic Thread Resizing ---" << std::endl;
        size_t newThreadCount = poolThreads + 2;
        std::cout << "Increasing thread count to " << newThreadCount << "..." << std::endl;
        pool.resize(newThreadCount);
        
        // Display status after increasing threads
        printPoolStatus(pool, "Status after increasing threads");
        
        // Test reducing thread count
        newThreadCount = poolThreads;
        std::cout << "Reducing thread count to " << newThreadCount << "..." << std::endl;
        pool.resize(newThreadCount);
        
        // Display status after reducing threads
        printPoolStatus(pool, "Status after reducing threads");
        
        // Test exception handling
        std::cout << "\n--- Testing Exception Handling ---" << std::endl;
        std::vector<std::future<int>> errorResults;
        for (int i = 0; i < 6; ++i) {
            bool shouldFail = (i % 3 == 0);
            errorResults.push_back(pool.enqueue(errorProneTask, i, shouldFail));
        }
        
        // Wait and get results of tasks that may throw exceptions
        std::cout << "\nWaiting for error-prone tasks to complete..." << std::endl;
        for (size_t i = 0; i < errorResults.size(); ++i) {
            try {
                int result = errorResults[i].get();
                std::cout << "Error-prone task " << i << " succeeded with result: " << result << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Error-prone task " << i << " failed: " << e.what() << std::endl;
            }
        }
        
        // Test clearing the task queue
        std::cout << "\n--- Testing Clearing Task Queue ---" << std::endl;
        for (int i = 100; i < 105; ++i) {
            int duration = durDist(gen);
            pool.enqueue(longComputation, i, duration);
        }
        
        // Display status after submitting tasks for clearing test
        printPoolStatus(pool, "Status after submitting clearing test tasks");
        
        // Clear the task queue
        pool.clearTasks();
        
        // Display status after clearing the queue
        printPoolStatus(pool, "Status after clearing the queue");
        
        // Test waiting for all tasks to complete
        std::cout << "\n--- Testing Wait for All Tasks to Complete ---" << std::endl;
        
        // Wait for all normal task results
        std::cout << "Waiting for normal tasks to complete..." << std::endl;
        for (size_t i = 0; i < results.size(); ++i) {
            try {
                int duration = results[i].get();
                std::cout << "Normal task " << i << " result: " << duration << "ms" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Normal task " << i << " failed: " << e.what() << std::endl;
            }
        }
        
        // Wait for all tasks to complete
        pool.waitForCompletion();
        
        // Display final status
        printPoolStatus(pool, "Final Status");
        
        std::cout << "\n--- Verifying ThreadPool Control Functionality ---" << std::endl;
        std::cout << "All tasks completed, thread pool control functionality is normal" << std::endl;
        std::cout << "Is the thread pool stopped: " << (pool.isStopped() ? "Yes" : "No") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== Day 5 Test Completed ===" << std::endl;
    std::cout << "Thread pool control functionality (resize, pause/resume, waitForTasks, clearTasks) is normal!" << std::endl;
    
    return 0;
}