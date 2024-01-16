#include "display.h"

#import <Cocoa/Cocoa.h>

void get_main_display_frame(int* x, int* y, int* w, int* h) {
    NSRect frame = [[NSScreen mainScreen] frame];
    NSRect visible = [[NSScreen mainScreen] visibleFrame];

    *x = visible.origin.x;
    *y = frame.size.height - (visible.size.height + visible.origin.y);
    *w = visible.size.width;
    *h = visible.size.height;
}