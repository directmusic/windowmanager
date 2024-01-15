project(JoesWindowManager CXX)

set(CXX_STANDARD 20)

add_executable(JoesWindowManager src/main.cpp)

target_link_libraries(JoesWindowManager "-framework ApplicationServices -framework CoreFoundation")
