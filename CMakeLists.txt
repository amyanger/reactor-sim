cmake_minimum_required(VERSION 3.10)
project(ReactorSim VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source files
set(SOURCES
    main.cpp
    Reactor_impl.cpp
    PhysicsEngine.cpp
    UI.cpp
    GameController.cpp
    RandomEvents.cpp
)

# Header files
set(HEADERS
    Reactor.h
    PhysicsEngine.h
    UI.h
    GameController.h
    RandomEvents.h
    Constants.h
)

# Add executable
add_executable(reactor ${SOURCES} ${HEADERS})


# Compiler-specific flags
if(MSVC)
    target_compile_options(reactor PRIVATE /W4)
else()
    target_compile_options(reactor PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Debug/Release configurations
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(reactor PRIVATE DEBUG)
endif()

# Installation rules
install(TARGETS reactor DESTINATION bin)

# Add custom target to run the simulator
add_custom_target(run
    COMMAND reactor
    DEPENDS reactor
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running reactor simulator..."
)