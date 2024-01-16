#include "rect.hpp"
#include "tree.hpp"
#include <ApplicationServices/ApplicationServices.h>

#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>

int main() {

    // Get the screen bounds
    CGDirectDisplayID main_display_id = CGMainDisplayID();

    // Get the width and height of the main display
    size_t width = CGDisplayPixelsWide(main_display_id);
    size_t height = CGDisplayPixelsHigh(main_display_id);

    BSP m({ 0, 0, static_cast<float>(width), static_cast<float>(height) });

    // Step 1: Obtain the list of all windows
    CFArrayRef window_list = CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly, kCGNullWindowID);

    // Step 2: Loop through the window list
    for (CFIndex i = 0; i < CFArrayGetCount(window_list); i++) {
        CFDictionaryRef window = (CFDictionaryRef)CFArrayGetValueAtIndex(window_list, i);

        // Skip system windows and menu bar items
        int32_t layer;
        CFNumberRef layer_num = (CFNumberRef)CFDictionaryGetValue(window, kCGWindowLayer);
        if (layer_num && CFNumberGetValue(layer_num, kCFNumberSInt32Type, &layer)) {
            if (layer != 0) { // Assuming menu bar items are on a non-zero layer
                continue; // Skip this window
            }
        }

        // Get the window ID
        CGWindowID window_id;
        CFNumberRef window_id_ref = (CFNumberRef)CFDictionaryGetValue(window, kCGWindowNumber);
        if (window_id_ref) {
            CFNumberGetValue(window_id_ref, kCFNumberIntType, &window_id);
        }

        // Get the PID
        pid_t pid;
        CFNumberRef pid_ref = (CFNumberRef)CFDictionaryGetValue(window, kCGWindowOwnerPID);
        if (pid_ref) {
            CFNumberGetValue(pid_ref, kCFNumberIntType, &pid);
        }

        // Get window bounds
        CGRect window_bounds;
        CFDictionaryRef bounds_dict = (CFDictionaryRef)CFDictionaryGetValue(window, kCGWindowBounds);
        if (bounds_dict) {
            CGRectMakeWithDictionaryRepresentation(bounds_dict, &window_bounds);
        }

        CFStringRef app = (CFStringRef)CFDictionaryGetValue(window, kCGWindowOwnerName);
        if (app) {
            char owner[256];
            CFStringGetCString(app, owner, 256, kCFStringEncodingUTF8);
        }

        CFStringRef window_title = (CFStringRef)CFDictionaryGetValue(window, kCGWindowName);
        char title[256];
        if (window_title) {
            CFStringGetCString(window_title, title, 256, kCFStringEncodingUTF8);
        }

        WindowInfo window_info = { window_title ? title : "", window_id, pid };
        // m->insert(window_info);
        if (std::string(title) != "StatusIndicator") {
            std::cout << "Inserting " << title << std::endl;
            m.insert(window_info);
        }
    }
    m.print_tree();
    // m->divide_evenly();
    // m->apply_new_bounds();
    // m->print_tree_visualization();

    // Release the window list
    CFRelease(window_list);

    return 0;
}
