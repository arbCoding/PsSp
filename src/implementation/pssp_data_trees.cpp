#include "pssp_data_trees.hpp"
#include <exception>

namespace pssp
{
//-----------------------------------------------------------------------------
// Functions that act on an NTreeStruct
//-----------------------------------------------------------------------------

//------------------------------------------------------------------------
// Create a group-node
//------------------------------------------------------------------------
std::unique_ptr<NTreeNode> create_group_node(const std::string& group_name)
{
    return std::make_unique<NTreeNode>(group_name, -1);
}
//------------------------------------------------------------------------
// End Create a group-node
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Create a leaf-node
//------------------------------------------------------------------------
std::unique_ptr<NTreeNode> create_leaf_node(const std::string& leaf_name, const int leaf_data_id)
{
    return std::make_unique<NTreeNode>(leaf_name, leaf_data_id);
}
//------------------------------------------------------------------------
// End Create a leaf-node
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Print the node information (name/value) as appropriate
//------------------------------------------------------------------------
void print_node(const NTreeNode* node)
{
    std::ostringstream oss{};
    if (node->data_id == -1) { oss << "Group: " << node->name << '\n'; }
    else { oss << "Name: " << node->name << ", ID: " << node->data_id << '\n'; }
    std::cout << oss.str();
}
//------------------------------------------------------------------------
// End Print the node information (name/value) as appropriate
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Traverse the tree and print out the names/values in pre-order
//------------------------------------------------------------------------
void print_preorder(const std::unique_ptr<NTreeNode>& node)
{
    if (!node) { return; }
    print_node(node.get());
    print_preorder(node->child_node);
    print_preorder(node->sibling_node);
}
//------------------------------------------------------------------------
// End Traverse the tree and print out the names/values in pre-order
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Traverse the tree and print out the names/values in order
//------------------------------------------------------------------------
void print_inorder(const std::unique_ptr<NTreeNode>& node)
{
    if (!node) { return; }
    print_inorder(node->child_node);
    print_node(node.get());
    print_inorder(node->sibling_node);
}
//------------------------------------------------------------------------
// End Traverse the tree and print out the names/values in order
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Traverse the tree and print out the names/values in post-order
//------------------------------------------------------------------------
void print_postorder(const std::unique_ptr<NTreeNode>& node)
{
    if (!node) { return; }
    print_postorder(node->child_node);
    print_postorder(node->sibling_node);
    print_node(node.get());
}
//------------------------------------------------------------------------
// End Traverse the tree and print out the names/values in post-order
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Traverse the tree and print out the names/values in level-order
//------------------------------------------------------------------------
void print_levelorder(const std::unique_ptr<NTreeNode>& node)
{
    if (!node) { return; }
    // We're going to queue the pointers
    // The queue doesn't own any of the pointers, so we use raw pointers
    std::queue<const NTreeNode*> q{};
    // Add the current node to the queue
    q.push(node.get());
    while (!q.empty())
    {
        const NTreeNode* current_node{q.front()};
        q.pop();
        print_node(current_node);
        if (current_node->child_node) { q.push(current_node->child_node.get()); }
        if (current_node->sibling_node) { q.push(current_node->sibling_node.get()); }
    }
}
//------------------------------------------------------------------------
// End Traverse the tree and print out the names/values in level-order
//------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// End Functions that act on an NTreeStruct
//-----------------------------------------------------------------------------
}