project(JoesWindowManager CXX OBJC)

add_executable(JoesWindowManager src/main.cpp src/display.m)
set_property(TARGET JoesWindowManager PROPERTY CXX_STANDARD 20)

target_link_libraries(JoesWindowManager "-framework ApplicationServices -framework CoreFoundation -framework Cocoa" objc)
