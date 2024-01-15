#include "rect.hpp"
#include "tree.hpp"
#include <ApplicationServices/ApplicationServices.h>

#include <map>
#include <memory>
#include <optional>
#include <string>

class TilingWindowManager {
private:
    std::shared_ptr<TreeNode> root;

    std::shared_ptr<TreeNode> find_leaf_to_split() {
        // Implement logic to find a suitable leaf for splitting
        // This can be as simple or as complex as needed
        // For now, let's assume it returns the first leaf found
        std::shared_ptr<TreeNode> current = root;
        while (!current->is_leaf()) {
            current = current->left; // or some other logic to choose a leaf
        }
        return current;
    }

public:
    TilingWindowManager(const WmRect& screenBounds) {
        root = std::make_shared<TreeNode>(screenBounds);
    }

    void add_window(std::unique_ptr<WindowInfo> window) {
        auto leaf_to_split = find_leaf_to_split();
        if (leaf_to_split) {
            // Split the leaf and add the window
            // Here you can define how to split (horizontally, vertically, etc.)
            // For simplicity, let's split vertically
            WmRect left_bounds = leaf_to_split->bounds; // Modify to represent the left half
            WmRect right_bounds = leaf_to_split->bounds; // Modify to represent the right half

            leaf_to_split->left = std::make_shared<TreeNode>(left_bounds);
            leaf_to_split->right = std::make_shared<TreeNode>(right_bounds);
            leaf_to_split->left->parent = leaf_to_split;
            leaf_to_split->right->parent = leaf_to_split;

            // Assign the window to one of the new leaves
            leaf_to_split->left->window = std::move(window);
        }
    }
};

void set_window_rect(WindowInfo& window, WmRect newPosition) {
    CGPoint new_origin = CGPointMake(newPosition.x, newPosition.y);
    CGSize new_size = CGSizeMake(newPosition.w, newPosition.h);

    AXUIElementRef app = AXUIElementCreateApplication(window.pid);
    AXUIElementRef window_ref = NULL;
    AXUIElementCopyAttributeValue(app, kAXFocusedWindowAttribute, (CFTypeRef*)&window_ref);

    AXValueRef origin_value = AXValueCreate(kAXValueCGPointType, (const void*)&new_origin);
    AXUIElementSetAttributeValue(window_ref, kAXPositionAttribute, origin_value);
    CFRelease(origin_value);

    AXValueRef size_value = AXValueCreate(kAXValueCGSizeType, (const void*)&new_size);
    AXUIElementSetAttributeValue(window_ref, kAXSizeAttribute, size_value);
    CFRelease(size_value);
}

void get_window_list() {
}

int main() {
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
            printf("Window ID: %d\n", (int)window_id);
        }

        // Get the PID
        pid_t pid;
        CFNumberRef pid_ref = (CFNumberRef)CFDictionaryGetValue(window, kCGWindowOwnerPID);
        if (pid_ref) {
            CFNumberGetValue(pid_ref, kCFNumberIntType, &pid);
            printf("Process ID: %d\n", (int)pid);
        }

        // Get window bounds
        CGRect window_bounds;
        CFDictionaryRef bounds_dict = (CFDictionaryRef)CFDictionaryGetValue(window, kCGWindowBounds);
        if (bounds_dict) {
            CGRectMakeWithDictionaryRepresentation(bounds_dict, &window_bounds);

            // Print window dimensions: origin (x,y) and size (width, height)
            printf("Window Origin: (%f, %f), Size: (%f, %f)\n",
                   window_bounds.origin.x, window_bounds.origin.y,
                   window_bounds.size.width, window_bounds.size.height);
        }

        CFStringRef app = (CFStringRef)CFDictionaryGetValue(window, kCGWindowOwnerName);
        if (app) {
            char title[256];
            CFStringGetCString(app, title, 256, kCFStringEncodingUTF8);
            // bool in_ignore_list = false;
            // for (int i = 0; i < sizeof(ignore_list) / sizeof(ignore_list[0]); i++) {
            //     if (strcmp(title, ignore_list[i]) == 0) {
            //         printf("title: %s, ignore_list[i]: %s\n", title, ignore_list[i]);
            //         in_ignore_list = true;
            //         continue;
            //     }
            // }
            // if (in_ignore_list) continue;
            printf("       owner: %s\n", title);
        }

        CFStringRef window_title = (CFStringRef)CFDictionaryGetValue(window, kCGWindowName);
        if (window_title) {
            char title[256];
            CFStringGetCString(window_title, title, 256, kCFStringEncodingUTF8);
            printf("Window Title: %s\n", title);
            if (std::string(title) == "MiniMeters") {
                printf("found MiniMeters\n");
                WmRect new_position = { 0, 0, 100, 100 };
                WindowInfo temp = { window_id, pid, new_position };
                set_window_rect(temp, new_position);
            }
        }

        WindowInfo window_info = { window_id, pid, { (int)window_bounds.origin.x, (int)window_bounds.origin.y, (int)window_bounds.size.width, (int)window_bounds.size.height } };
    }

    // Release the window list
    CFRelease(window_list);

    WmRect screen_bounds = { 0, 0, 1920 * 2, 1080 * 2 };
    TilingWindowManager* m = new TilingWindowManager(screen_bounds);

    get_window_list();
    return 0;
}
