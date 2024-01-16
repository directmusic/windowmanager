#pragma once

struct WmRect {
    int x, y, w, h;

    WmRect left_half() {
        return { x, y, w / 2, h };
    }
    WmRect right_half() {
        return { x + w / 2, y, w / 2, h };
    }
    WmRect top_half() {
        return { x, y, w, h / 2 };
    }
    WmRect bottom_half() {
        return { x, y + h / 2, w, h / 2 };
    }
    bool contains(const WmRect& other) {
        return x <= other.x && y <= other.y && x + w >= other.x + other.w && y + h >= other.y + other.h;
    }
};