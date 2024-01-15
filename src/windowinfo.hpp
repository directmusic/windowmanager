#pragma once
#include "rect.hpp"
#include <ApplicationServices/ApplicationServices.h>

struct WindowInfo {
    CGWindowID window_id;
    pid_t pid;
    WmRect rect;
};