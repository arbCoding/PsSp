#ifndef PSSP_DATA_TREES_HPP_20230610
#define PSSP_DATA_TREES_HPP_20230610

//-----------------------------------------------------------------------------
// Include statements
//-----------------------------------------------------------------------------
// Standard Library stuff, https://en.cppreference.com/w/cpp/standard_library
#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <queue>
//-----------------------------------------------------------------------------
// End Include statements
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Description
//-----------------------------------------------------------------------------
// Data can be grouped into a tree-like structure. Composed of group-nodes
//  and leaf-nodes.
//
// Group-nodes are purely for organizational purposes (grouping data)
//  while Leaf-nodes are purely for holding data.
//
// In the case for PsSp, the data in a leaf-node will be a unique data_id
//  for each seismogram (just the integer data_id, the data is elsewhere
//  and the data_id allows us to find it)
//
// While the group-nodes allow us to provide arbitrary grouping ability
//
// Common kinds of groups:
// 1) If a seismic station is 3 component, then we can group all of its seismograms
//  into a single station
//      station1->components
//      station2->components
//      ...
//      stationN->components
// 2) If an array has n stations, then we can group all of its stations into
//  a single array
//      array1->stations->components
//      array2->stations->components
//      ...
//      arrayN->stations->components
// 3) If there are multiple earthquakes, each observed by the same array
//  then we can form groups of
//      event1->array->stations->components
//      event2->array->stations->components
//      ...
//      eventN->array->stations->components
// 4) If a station observes multiple earthquakes, each seismogram can be
//  group by the earthquake
//      station1->earthquake->components
//      station2->earthquake->components
//      ...
//      station3->earthquake->components
//
// Or whatever we want. The point is to be flexible.
//
// To that end, we need a way to distinguish between group-nodes and leaf-nodes
//  Since group nodes don't really count as unique data, just data groups
//  If a group-node, then the data_id is -1 (because no real piece of data
//  can ever have that value)
//
// We also need a way for nodes to hold different types of "data"
//  e.g. a group-node can hold a std::string for the group name
//          while a leaf-node can hold an int for the unique data_id (and a std::string for a name)
//
// std::variant might be a useful way for doing this, but I have a hard time working
//  with it. For now, I'll just have it so each node has a field for a string and for an
//  int. Then the logic is that if it is a group-node the int is (-1) but the
//  string is the group-name, if it is a leaf-node the string is some name but the int
//  is the unique data_id
//
// For the sake of avoiding memory-leaks, I'm going to use smart pointers
//  in particular, I think it makes sense to use std::unique_ptr since a child-node
//  shouldn't have multiple parent nodes.
//
// Because doing it with smart pointers can be a bit of a pain, I think that
//  rearranging of a tree (or subtree) should be done by destorying and rebuilding
//  the tree rather than inserting/moving/etc. This shouldn't be a huge issue
//  because the data contained in the tree itself is fairly small (names of groups
//  and ints for data_ids instead of SacStreams) and because I don't expect
//  a tree to get particularly large.
//
// Source for information on USArray: https://www.ds.iris.edu/mda/TA/
//
// What do I mean by not too large? Let's make an absurd order-of-magnitude calculation
//
// The USArray had a total of 1892 deployments (stations were redeployed in
//  a staggered manner over time to allow for this many deployments with far
//  fewer instruments). Let's say each station is 3-components and the analyst
//  wants to look at 100 earthquakes per station.
//
// That amounts to 1892 * 3 * 100 = 567,600 unique data_ids in the tree
//
// The typical 1 hour seismogram, recorded at 40 Hz, is ~1Mb in size (the actual
//  SAC-file is smaller because they use floats for the timeseries and we use
//  doubles)
//
// So the footprint of having all those seismograms in memory at once would be
//  567.6 Gb, which is tremendously over what any user's desktop has available.
// And hence it makes sense for them to use multiple separate projects
//  to manage the data (or for me to implement a way of only maintaining
//  a fraction of the data in memory and balancing loading and unloading
//  data as needed, which is certainly a possibility)
//
// Even then, that is a hugely ambitious project. Even if we only wanted
//  to use 10 minutes of data instead of an hour (so 1/6 the previous estimate
//  ~94.6 Gb) or 1 minute of data (a further 1/10, ~9.5 Gb).
//
// Most passive-source projects are lucky if they have 1/10-to-1/20 that many
//  deployments.
//
//=============================================================================
// Though now that I'm thinking about this, I should handle how much data is 
//  kept in memory and hot load/unload as needed to allow for massive datasets.
//
// But that sort of balancing is a project for later. I just don't want to
//  forget this line of thought.
//
// It might be fine to load data into memory on the fly as needed, and then store
//  it in the project database in a temporary table that gets deleted on program
//  closure?
//
// Added benefit of possibly restoring state if program crashes?
//
// Alternatively, could do data chunking (load some amount of seismograms at once
//  unload that many at once and replace with that many new?). I'll need to
//  ponder that more later, for now my focus is elsewhere (N-ary trees).
//=============================================================================
//-----------------------------------------------------------------------------
// End Description
//-----------------------------------------------------------------------------

namespace pssp
{
//-----------------------------------------------------------------------------
// NTreeStruct
//-----------------------------------------------------------------------------
struct NTreeNode
{
    // Name is always used (group-name, file-name, station-name, whatever)
    std::string name{};
    // Default is -1, which means it is a group (data have data_ids > 0)
    int data_id{-1};
    // Pointer to sibling node
    // Sibling nodes exist at the same depth-level in the tree (same group)
    std::unique_ptr<NTreeNode> sibling_node{};
    // Pointer to child node
    // Child nodes are down an additional level (sub-group)
    std::unique_ptr<NTreeNode> child_node{};
    // Constructor
    NTreeNode(const std::string& name_, const int data_id_) : name(name_), data_id(data_id_) {}
    // Destructor that prints-out on destruction for debugging purposes
    // Excellent, now we can confirm destruction quite easily, the
    //  unique_ptrs are working their magic!
    //~NTreeNode() { std::cout << "Destroying node: " << name << '\n'; }
};
//-----------------------------------------------------------------------------
// End NTreeStruct
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions that act on an NTreeStruct
//-----------------------------------------------------------------------------
// Create a group-node
std::unique_ptr<NTreeNode> create_group_node(const std::string& group_name);
// Create a leaf-node
std::unique_ptr<NTreeNode> create_leaf_node(const std::string& leaf_name, const int leaf_data_id);
// Print the node information (name/value) as appropriate
// This doesn't care about ownership, so we use raw pointers to make life easier
void print_node(const NTreeNode* node);
// These generally don't care about ownership, because they're not transferring it
// But we maintain the unique_ptrs so I don't need to have '.get()' all over the place
// The point of these is to layout the algorithms for traversal and to allow me to
//  look at the structure and gain a sense for what it is
// Traverse the tree and print out the names/values in pre-order
void print_preorder(const std::unique_ptr<NTreeNode>& node);
// Traverse the tree and print out the names/values in order
void print_inorder(const std::unique_ptr<NTreeNode>& node);
// Traverse the tree and print out the names/values in post-order
void print_postorder(const std::unique_ptr<NTreeNode>& node);
// Traverse the tree and print out the names/values in level-order
void print_levelorder(const std::unique_ptr<NTreeNode>& node);
//-----------------------------------------------------------------------------
// End Functions that act on an NTreeStruct
//-----------------------------------------------------------------------------
}

#endif
