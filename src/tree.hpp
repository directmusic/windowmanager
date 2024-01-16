#include "rect.hpp"
#include "windowinfo.hpp"

#include <iostream>
#include <memory>
#include <queue>

#define USE_UNDOCUMENTED_API

#if defined(USE_UNDOCUMENTED_API)
extern "C" AXError _AXUIElementGetWindow(AXUIElementRef, CGWindowID* out);
#endif

bool set_window_rect(const WindowInfo& window, WmRect new_pos) {
    std::cout << "Setting window rect to "
              << "x: " << new_pos.x << ", y: " << new_pos.y << ", w: " << new_pos.w << ", h:" << new_pos.h << std::endl;
    CGPoint new_origin = CGPointMake(new_pos.x, new_pos.y);
    CGSize new_size = CGSizeMake(new_pos.w, new_pos.h);

    AXUIElementRef app = AXUIElementCreateApplication(window.pid);
    if (!app) {
        std::cerr << "Could not create AXUIElement for PID " << window.pid << std::endl;
        return false;
    }

    CFArrayRef windows;
    AXError error = AXUIElementCopyAttributeValues(app, kAXWindowsAttribute, 0, 100, &windows);
    if (error != kAXErrorSuccess) {
        std::cerr << "Could not get windows for app " << window.title << std::endl;
        CFRelease(app);
        return false;
    }

    for (CFIndex i = 0; i < CFArrayGetCount(windows); i++) {
        AXUIElementRef window_ref = (AXUIElementRef)CFArrayGetValueAtIndex(windows, i);
#if defined(USE_UNDOCUMENTED_API)
        CGWindowID id;
        _AXUIElementGetWindow(window_ref, &id);

        if (id == window.window_id) {
            AXValueRef origin_value = AXValueCreate((AXValueType)kAXValueCGPointType, &new_origin);
            AXUIElementSetAttributeValue(window_ref, kAXPositionAttribute, origin_value);
            CFRelease(origin_value);

            AXValueRef size_value = AXValueCreate((AXValueType)kAXValueCGSizeType, &new_size);
            AXUIElementSetAttributeValue(window_ref, kAXSizeAttribute, size_value);
            CFRelease(size_value);
        }
#else
        CFStringRef title_ref;
        AXUIElementCopyAttributeValue(window_ref, kAXTitleAttribute, (CFTypeRef*)&title_ref);
        if (title_ref) {
            char title[256];
            CFStringGetCString(title_ref, title, 256, kCFStringEncodingUTF8);
            if (std::string(title) == window.title) {
                AXValueRef origin_value = AXValueCreate((AXValueType)kAXValueCGPointType, &new_origin);
                AXUIElementSetAttributeValue(window_ref, kAXPositionAttribute, origin_value);
                CFRelease(origin_value);

                AXValueRef size_value = AXValueCreate((AXValueType)kAXValueCGSizeType, &new_size);
                AXUIElementSetAttributeValue(window_ref, kAXSizeAttribute, size_value);
                CFRelease(size_value);
            }
        }
#endif
    }

    if (windows)
        CFRelease(windows);
    if (app)
        CFRelease(app);
    return true;
}

struct Rectangle {
    float x, y, width, height;

    Rectangle(float x, float y, float width, float height)
        : x(x)
        , y(y)
        , width(width)
        , height(height) { }

    // Helper function to split the rectangle
    std::pair<Rectangle, Rectangle> split(bool vertical) const {
        if (vertical) {
            float half_width = width / 2;
            return { Rectangle(x, y, half_width, height), Rectangle(x + half_width, y, half_width, height) };
        } else {
            float half_height = height / 2;
            return { Rectangle(x, y, width, half_height), Rectangle(x, y + half_height, width, half_height) };
        }
    }

    void print() {
        std::cout << "Bounds: " << x << " " << y << " " << width << " " << height << std::endl;
    }
};

class BSPNode {
public:
    Rectangle rect;
    std::optional<WindowInfo> window = std::nullopt;
    std::vector<std::shared_ptr<BSPNode>> children;
    bool splitVertical = true;

    BSPNode(Rectangle r)
        : rect(r) { }
};

class BSP {
private:
    std::shared_ptr<BSPNode> root;

public:
    BSP(Rectangle bounds) {
        root = std::make_shared<BSPNode>(bounds);
    }

    void insert(WindowInfo window) {
        insert(root, window, 0);
    }

    void insert(std::shared_ptr<BSPNode> node, WindowInfo window, int depth) {
        if (!node->window && node->children.empty()) {
            node->window = window;
            std::cout << " Initial push" << std::endl;
            node->rect.print();
            return;
        }

        if (node->window && node->children.empty()) {
            std::cout << "moving window to first child" << std::endl;
            node->rect.print();

            auto left_node = std::make_shared<BSPNode>(node->rect);
            auto right_node = std::make_shared<BSPNode>(node->rect);

            if (depth % 2 == 0) {
                std::cout << "HORIZONTAL" << std::endl;
                left_node->rect.width /= 2;
                right_node->rect.x += node->rect.width / 2;
                right_node->rect.width = node->rect.width / 2;
            } else {
                std::cout << "VERTICAL" << std::endl;
                left_node->rect.height /= 2;
                right_node->rect.y += node->rect.height / 2;
                right_node->rect.height = node->rect.height / 2;
            }

            left_node->window = node->window;
            right_node->window = window;

            node->children.push_back(left_node);
            node->children.push_back(right_node);

            node->window = std::nullopt;
        } else {
            bool left_node_is_bigger = node->children.front()->children.size() > node->children.back()->children.size();

            if (left_node_is_bigger)
                insert(node->children.back(), window, depth + 1);
            else if (!left_node_is_bigger) {
                insert(node->children.front(), window, depth + 1);
            } else {
                std::cout << "uh oh" << std::endl;
            }
        }
    }

private:
    void print_tree(const std::shared_ptr<BSPNode>& node, int level = 0) const {
        if (node == nullptr) {
            return;
        }

        if (node->window) {
            std::cout << "Level: " << level << std::endl;
            set_window_rect(node->window.value(), { static_cast<int>(node->rect.x), static_cast<int>(node->rect.y), static_cast<int>(node->rect.width), static_cast<int>(node->rect.height) });
        }
        // if (node->window) {
        //     std::cout << "Level " << level << " - Rectangle: ("
        //               << node->rect.x << ", " << node->rect.y << ", "
        //               << node->rect.width << ", " << node->rect.height
        //               << "), Item ID: " << node->window->title << std::endl;
        // }

        // Recursively print left and right children
        if (node->children.size() > 0)
            print_tree(node->children[0], level + 1);
        if (node->children.size() > 1)
            print_tree(node->children[1], level + 1);
    }

public:
    void print_tree() const {
        print_tree(root);
    }
};