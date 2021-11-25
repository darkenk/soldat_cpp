set(SRC
    linux64/Threads.cpp
)

add_library(PlatformLib ${SRC})
target_include_directories(PlatformLib PRIVATE include)

