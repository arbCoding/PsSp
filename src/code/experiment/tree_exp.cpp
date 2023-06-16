#include "pssp_data_trees.hpp"
#include <iostream>
#include <string>

int main()
{
    // For now, dumb manual way of doing it
    // Create the tree structure
    std::unique_ptr<pssp::NTreeNode> root_node = pssp::create_root_node();
    root_node->child_node = pssp::create_group_node("Group 1", {0});
    root_node->child_node->child_node = pssp::create_leaf_node("Leaf1", {0}, 0);
    root_node->child_node->sibling_node = pssp::create_leaf_node("Leaf2", {0}, 1);
    root_node->sibling_node = pssp::create_group_node("Group 2", {1});
    root_node->sibling_node->child_node = pssp::create_group_node("Group 3", {2});
    root_node->sibling_node->child_node->child_node = pssp::create_leaf_node("Leaf3", {2}, 2);
    root_node->sibling_node->child_node->sibling_node = pssp::create_leaf_node("Leaf4", {2}, 3);

    std::cout << "Pre-Order:\n";
    pssp::print_preorder(root_node);
    std::cout << "\nIn-Order:\n";
    pssp::print_inorder(root_node);
    std::cout << "\nPost-Order:\n";
    pssp::print_postorder(root_node);
    std:: cout << "\nLevel-Order:\n";
    pssp::print_levelorder(root_node);
    // Now let's destroy a node to see what happens
    std::cout << "\nDestorying a child_node\n";
    root_node->child_node.reset();
    std:: cout << "\nLevel-Order:\n";
    pssp::print_levelorder(root_node);
    return 0;
}