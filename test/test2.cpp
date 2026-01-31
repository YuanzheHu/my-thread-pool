#include <iostream>
#include <chrono>
#include <thread>
#include "ThreadPool.h"

// Simple sleep function to simulate a task
void sleepFor(int seconds) {
    std::cout << "Starting sleep for " << seconds << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    std::cout << "Sleep for " << seconds << " seconds ended!" << std::endl;
}

int main() {
    std::cout << "=== C++11 Thread Pool Implementation - Day 2 Test ===" << std::endl;
    std::cout << "Creating thread pool..." << std::endl;
    
    // Get the number of CPU cores
    size_t threadCount = std::thread::hardware_concurrency();
    std::cout << "The system has " << threadCount << " CPU cores" << std::endl;
    
    // Ensure at least one thread
    threadCount = threadCount == 0 ? 1 : threadCount;
    
    // To better observe the effect, we use a smaller number of threads
    size_t poolThreads = std::min(threadCount, (size_t)4);
    
    try {
        std::cout << "\n--- Testing Thread Pool Creation ---" << std::endl;
        // Create thread pool
        ThreadPool pool(poolThreads);
        
        std::cout << "Thread pool created successfully!" << std::endl;
        std::cout << "The thread pool has " << poolThreads << " worker threads" << std::endl;
        
        // Test thread pool state
        // std::cout << "\n--- Testing Thread Pool State ---" << std::endl;
        // std::cout << "Is the thread pool stopped: " << (pool.isStopped() ? "Yes" : "No") << std::endl;
        
        // Let the main thread wait for a while to observe the behavior of worker threads
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        std::cout << "\n--- Preparing to destroy the thread pool ---" << std::endl;
        std::cout << "Note: On the third day, we will implement task submission functionality" << std::endl;
        std::cout << "Currently, the thread pool can only create worker threads but cannot submit tasks yet" << std::endl;
        
        // The thread pool will be automatically destroyed when it goes out of scope
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n--- Test Completed ---" << std::endl;
    std::cout << "Main function ends, thread pool has been destroyed" << std::endl;
    
    return 0;
}