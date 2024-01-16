project(JoesWindowManager CXX)

add_executable(JoesWindowManager src/main.cpp)
set_property(TARGET JoesWindowManager PROPERTY CXX_STANDARD 20)

target_link_libraries(JoesWindowManager "-framework ApplicationServices -framework CoreFoundation")
