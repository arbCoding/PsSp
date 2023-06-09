#include "pssp_nary_tree.hpp"
#include <variant>
#include <vector>
#include <string>
#include <iostream>

// This is a play ground for working on the n-ary tree implementation.
// I'll cleanup and improve stuff later, this already took a lot
// longer than it should have to make it nice and generic/templated

// To compile this
// clang++ -std=c++20 -pedantic-errors -Wall -Wextra -Werror -Wshadow -ggdb -I./src/header/ nary_tree_example.cpp -o test

int main()
{
    using namespace pssp;
    // Build the N-ary tree manually
    NodePtr<int, std::string> root = std::make_unique<NTreeNode<int, std::string>>(1);

    root->child = std::make_unique<NTreeNode<int, std::string>>("Hello");
    root->child->sibling = std::make_unique<NTreeNode<int, std::string>>(2);
    root->child->sibling->sibling = std::make_unique<NTreeNode<int, std::string>>("World");
    root->child->sibling->sibling->sibling = std::make_unique<NTreeNode<int, std::string>>(3);

    // Traverse and print the N-ary tree
    std::cout << std::get<int>(root->data) << std::endl;
    if (root->child)
        std::cout << std::get<std::string>(root->child->data) << std::endl;
    if (root->child && root->child->sibling)
        std::cout << std::get<int>(root->child->sibling->data) << std::endl;
    if (root->child && root->child->sibling && root->child->sibling->sibling)
        std::cout << std::get<std::string>(root->child->sibling->sibling->data) << std::endl;
    if (root->child && root->child->sibling && root->child->sibling->sibling && root->child->sibling->sibling->sibling)
        std::cout << std::get<int>(root->child->sibling->sibling->sibling->data) << std::endl;
    return 0;
}