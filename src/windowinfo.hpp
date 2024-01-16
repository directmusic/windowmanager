#pragma once
#include "rect.hpp"
#include <ApplicationServices/ApplicationServices.h>
#include <string>

struct WindowInfo {
    std::string title;
    CGWindowID window_id;
    pid_t pid;
};
