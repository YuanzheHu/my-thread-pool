#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <thread>
#include <random>
#include "ThreadPool.h"

// test function that simulates a long computation
int longComputation(int duration) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    return duration;
}

// test function that throws an exception
int errorProneTask(int n) {
    if(n % 3 == 0) {
        throw std::runtime_error("Number is divisible by 3 !");
    }
    return n * n;
}

//print thread pool status
void printThreadPoolStatus(ThreadPool& pool, const std::string& stage) {
    std::cout << "Thread Pool Status at " << stage << ":" << std::endl;
    std::cout << "  Total Threads: " << pool.getThreadCount() << std::endl;
    std::cout << "  Active Threads: " << pool.getActiveThreadCount() << std::endl;
    std::cout << "  Waiting Threads: " << pool.getWaitingThreadCount() << std::endl;
    std::cout << "  Tasks in Queue: " << pool.getTaskCount() << std::endl;
    std::cout << "  Completed Tasks: " << pool.getCompletedTaskCount() << std::endl;
    std::cout << "  Failed Tasks: " << pool.getFailedTaskCount() << std::endl;
}


int main(){
    std::cout << "test_4" << std::endl;

    try{
        size_t threadCount = std::thread::hardware_concurrency();
        threadCount = threadCount == 0 ? 1 : threadCount;

        size_t poolThreads = std::min(threadCount, static_cast<size_t>(4));

        std::cout << "Creating thread pool with " << poolThreads << " threads" << std::endl;

        ThreadPool pool(poolThreads);

        // Initial Status
        printThreadPoolStatus(pool, "Initialization");

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100, 500);

        std::cout <<"\n Enqueuing 10 normal tasks" << std::endl;
        std::vector<std::future<int>> results;
        // Enqueue long computation tasks
        for(int i = 0; i < 10; ++i) {
            int duration = dis(gen);
            results.emplace_back(
                pool.enqueue(longComputation, duration));
        };
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
        printThreadPoolStatus(pool, "After Enqueuing Normal Tasks");

        std::cout << "\nEnqueued 10 error prone tasks"<< std::endl;
        std::vector<std::future<int>> errorResults;
        for(int i = 1; i <= 10; ++i) {
            errorResults.emplace_back(
                pool.enqueue(errorProneTask, i));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
        printThreadPoolStatus(pool, "After Enqueuing Error Prone Tasks");

        // Retrieve results of normal tasks
        std::cout << "\nRetrieving results of normal tasks:" << std::endl;
        for(size_t i = 0; i < results.size(); ++i) {
            try {
                int result = results[i].get();
                std::cout << "  Task " << i << " completed in " << result << " ms" << std::endl;
            } catch(const std::exception& e) {
                std::cout << "  Task " << i << " failed with exception: " << e.what() << std::endl;
            }
        }
        
        // Retrieve results of error prone tasks
        std::cout << "\nRetrieving results of error prone tasks:" << std::endl;
        for(size_t i = 0; i < errorResults.size(); ++i) {
            try {
                int result = errorResults[i].get();
                std::cout << "  Task " << i + 1 << " result: " << result << std::endl;
            } catch(const std::exception& e) {
                std::cout << "  Task " << i + 1 << " failed with exception: " << e.what() << std::endl;
            }
        }
        
        // test final status
        printThreadPoolStatus(pool, "After Retrieving All Results");

        std::cout<<"\n --- Verify Exception Handling ---"<<std::endl;
        std::cout << "all tasks have been processed, thread pool should be idle now." << std::endl;
        std::cout << "thread pool has been stopped." << (pool.isStopped() ? "Yes" : "No") << std::endl;

        // test atomic operations
        std::cout << "\nVerifying atomic operations:" << std::endl;
        std::cout << "  Active Threads: " << pool.getActiveThreadCount() << std::endl;
        std::cout << "  Waiting Threads: " << pool.getWaitingThreadCount() << std::endl;
        std::cout << "  Completed Tasks: " << pool.getCompletedTaskCount() << std::endl;

        // verify thread count
        std::cout << "\nVerifying thread count:" << std::endl;
        std::cout << "  Total Threads = Active Threads + Waiting Threads" << std::endl;
        std::cout << "  " << pool.getThreadCount() << " = " 
                  << pool.getActiveThreadCount() << " + " 
                  << pool.getWaitingThreadCount() << std::endl;
        
        size_t sum = pool.getActiveThreadCount() + pool.getWaitingThreadCount();
        if(sum != pool.getThreadCount()) {
            throw std::runtime_error("Thread count verification failed!");
        } else {
            std::cout << "Thread count verification passed!" << std::endl;
        }
        
    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "test_4 completed successfully" << std::endl;
    std::cout << "---------------------------------" << std::endl;

    return 0;
}