#ifndef PSSP_NARY_TREE_HPP
#define PSSP_NARY_TREE_HPP
//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
// smart pointers
#include <memory>
#include <vector>
#include <variant>
#include <iostream>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// To handle data organization in a more abstract sense
// We want to use an N-ary tree.
//
// To keep things efficient, we're going to represent the n-ary tree as a binary
//  tree (this is a well-known practice)
//
// That means we'll be able to use all the super-efficient binary tree functions
//  for traversal, depth measuring, insertion, deletion, sorting, etc.
//
// Because we don't really want anything to be private here (as far as I can tell)
//  I will be using a struct instead of a class
//
// Because there is a lot of template usage, for possibly generic stuff
//  I'm planning on leaving this as a header-only combo interface-implementation
// That breaks my usual rule of splitting those parts, but this is a special case
//
// Information on traversal functions can be found here:
//  https://www.geeksforgeeks.org/tree-traversals-inorder-preorder-and-postorder/
//
// Because I am using smart pointers (std::unique_ptr in this case) it is
//  really easy to safely destroy an n-ary tree, just destroy the unique_ptr
//  of the root
// e.g. root.reset(nullptr);
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

namespace pssp
{
    //-------------------------------------------------------------------------
    // NTreeNode struct
    //-------------------------------------------------------------------------
    // Empty so we can alias the NodePtr
    template<typename... Ts>
    struct NTreeNode;
    // Alias for brevity
    template<typename... Ts>
    using NodePtr = std::unique_ptr<NTreeNode<Ts...>>;
    // This is the actual N-ary tree (as a binary tree) with some helper functions
    // We allow it to have generic types at any node (or multiple types)
    template<typename... Ts>
    struct NTreeNode
    {
        // The data at this node
        std::variant<Ts...> data{};
        // Standard left-child
        NodePtr<Ts...> child;
        // Instead of a standard right-child, we instead have a sibling
        // So this is at the exact same depth as the present node
        NodePtr<Ts...> sibling;
        
        // Constructor
        NTreeNode(const std::variant<Ts...>& value) : data(value), child(nullptr), sibling(nullptr) {}
    };
    //-------------------------------------------------------------------------
    // End NTreeNode struct
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    // Additional Functions
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // End Additional Functions
    //-------------------------------------------------------------------------
}
#endif
