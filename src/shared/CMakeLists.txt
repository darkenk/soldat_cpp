find_package(fmt REQUIRED)


set(SRC 
  Cvar.cpp
  Command.cpp
  Game.cpp
  TraceLog.cpp
  Anims.cpp
)

add_library(shared STATIC ${SRC})
target_link_libraries(shared fmt::fmt)
target_include_directories(shared PRIVATE ../)
