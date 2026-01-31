#include <iostream>
#include <thread>
#include "ThreadPool.h"

int main() {
    std::cout << "Creating thread pool..." << std::endl;
    
    try {
        // Create a thread pool with 4 worker threads
        ThreadPool pool(4);
        
        std::cout << "Thread pool created successfully!" << std::endl;
        std::cout << "The thread pool has 4 worker threads" << std::endl;
        
        // Pause the program briefly to observe the output
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // At this point, we haven't implemented specific functionality, so the thread pool will be destroyed immediately
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Main function ends, thread pool has been destroyed" << std::endl;
    
    return 0;
}