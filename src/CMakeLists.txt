# CMakeLists.txt for the src directory
set(SOURCES
    ThreadPool.cpp
)

# Create thread pool library
add_library(threadpool ${SOURCES})

# Link thread pool library
find_package(Threads REQUIRED)
target_link_libraries(threadpool PRIVATE Threads::Threads)

# Install library
install(TARGETS threadpool
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
