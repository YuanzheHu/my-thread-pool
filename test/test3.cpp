#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include "ThreadPool.h"

// Test function: Calculate Fibonacci numbers
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

// Test function: Return a string
std::string getMessage(const std::string& name, int waitSeconds) {
    std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
    return "Hello, " + name + "! (waited " + std::to_string(waitSeconds) + "s)";
}

// Test function: No return value
void printMessage(const std::string& message) {
    std::cout << "Message: " << message << std::endl;
}

int main() {
    std::cout << "=== C++11 Thread Pool Implementation - Day 3 Test ===" << std::endl;
    
    try {
        // Create thread pool with number of threads equal to hardware concurrency
        size_t threadCount = std::thread::hardware_concurrency();
        threadCount = threadCount == 0 ? 1 : threadCount;
        
        // For better observation, use fewer threads
        size_t poolThreads = std::min(threadCount, (size_t)4);
        
        std::cout << "System has " << threadCount << " CPU cores" << std::endl;
        std::cout << "Creating thread pool with " << poolThreads << " threads" << std::endl;
        
        ThreadPool pool(poolThreads);
        
        // Storage for future objects
        std::vector<std::future<int>> fibs;
        std::vector<std::future<std::string>> msgs;
        std::vector<std::future<void>> prints;
        
        std::cout << "\n--- Submitting Tasks of Different Types ---" << std::endl;
        
        std::cout << "Submitting Fibonacci calculation tasks..." << std::endl;
        // Submit tasks for calculating Fibonacci numbers (with smaller numbers to avoid long computation times)
        for (int i = 20; i < 25; ++i) {
            fibs.push_back(
                pool.enqueue(fibonacci, i)
            );
        }
        
        std::cout << "Submitting message retrieval tasks..." << std::endl;
        // Submit tasks for retrieving messages
        for (int i = 1; i <= 3; ++i) {
            msgs.push_back(
                pool.enqueue(getMessage, "User" + std::to_string(i), 1)
            );
        }
        
        std::cout << "Submitting message printing tasks..." << std::endl;
        // Submit tasks for printing messages
        for (int i = 0; i < 3; ++i) {
            prints.push_back(
                pool.enqueue(printMessage, "This is message " + std::to_string(i))
            );
        }
        
        std::cout << "\n--- Retrieving Task Results ---" << std::endl;
        
        // Retrieve Fibonacci results
        std::cout << "Fibonacci results:" << std::endl;
        for (size_t i = 0; i < fibs.size(); ++i) {
            std::cout << "fibonacci(" << (i + 20) << ") = " << fibs[i].get() << std::endl;
        }
        
        // Retrieve message results
        std::cout << "\nMessage results:" << std::endl;
        for (auto& future : msgs) {
            std::cout << future.get() << std::endl;
        }
        
        // Wait for print tasks to complete
        std::cout << "\nWaiting for print tasks to complete..." << std::endl;
        for (auto& future : prints) {
            future.wait();
        }
        
        std::cout << "\n--- Test Complete ---" << std::endl;
        std::cout << "All tasks completed! Thread pool functioning normally" << std::endl;
        
        // Test thread pool status
        std::cout << "Is the thread pool stopped: " << (pool.isStopped() ? "Yes" : "No") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}