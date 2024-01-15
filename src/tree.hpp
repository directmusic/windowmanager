#include "rect.hpp"
#include "windowinfo.hpp"
#include <iostream>

struct TreeNode {
    WmRect bounds; // Defines the area of the screen the node covers
    std::shared_ptr<TreeNode> parent; // Pointer to the parent node
    std::shared_ptr<TreeNode> left; // Pointer to the left child (e.g., split vertically)
    std::shared_ptr<TreeNode> right; // Pointer to the right child (e.g., split horizontally)
    std::shared_ptr<WindowInfo> window; // Pointer to the window occupying this node (if any)

    TreeNode(const WmRect& bounds)
        : bounds(bounds)
        , parent(nullptr)
        , left(nullptr)
        , right(nullptr)
        , window(nullptr) { }

    bool is_leaf() const {
        return !left && !right;
    }
};