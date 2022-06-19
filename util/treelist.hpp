/**
  *  \file util/treelist.hpp
  *  \brief Class util::TreeList
  */
#ifndef C2NG_UTIL_TREELIST_HPP
#define C2NG_UTIL_TREELIST_HPP

#include <vector>
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Tree list.
        Provides a container of pairs of keys (integers) and strings, arranged in a hierarchical fashion.

        The tree can be traversed by iterating over a node's children:
        <code>for (size_t i = getFirstChild(n); i != nil; i = getNextSibling(i))</code>

        A tree always contains a root node. */
    class TreeList {
     public:
        /** Node index signifying "no such node".
            This value is returned by methods to report that the given node (e.g. first child) does not exist. */
        static const size_t nil = static_cast<size_t>(-1);

        /** Node index of root node. */
        static const size_t root = 0;

        /** Constructor.
            Makes an empty list. */
        TreeList();

        /** Destructor. */
        ~TreeList();

        /** Add a key/string pair.
            @param key      Key
            @param s        String
            @param childOf  Add as child of this node
            @return Index of newly-created node */
        size_t add(int32_t key, const String_t& s, size_t childOf);

        /** Add a key/string pair, given a path.
            Nodes are identified by their name on the path (exact string comparison).
            If a node on the path does not exist, it is created with Id 0.
            If the final node already exists, its key is just replaced; no additional node is added.
            Consequentially, if an empty path is specified, this function just updates childOf's key.

            @param key      Key
            @param path     List of strings
            @param childOf  Starting node */
        size_t addPath(int32_t key, afl::base::Memory<const String_t> path, size_t childOf);

        /** Swap content with another list.
            @param other Other list */
        void swap(TreeList& other);

        /** Clear list. */
        void clear();

        /** Check whether a node has children.
            @param index Index
            @return true if node has children */
        bool hasChildren(size_t index) const;

        /** Get first child of a node.
            @param index Index
            @return First child index, nil if none */
        size_t getFirstChild(size_t index) const;

        /** Get next sibling of a node.
            @param index Index
            @return Next sibling index, nil if none */
        size_t getNextSibling(size_t index) const;

        /** Find child, given a label.
            @param s      Label to find
            @param parent Search this node's children
            @return Found child index, nil if not found */
        size_t findChildByLabel(const String_t& s, size_t parent) const;

        /** Get key/string pair, given an index.
            @param [in]  index  Index
            @param [out] key    Key
            @param [out] s      String
            @retval true Index valid; key/s have been set
            @retval false Index out of range; key/s not modified */
        bool get(size_t index, int32_t& key, String_t& s) const;

     private:
        struct Element {
            int32_t key;
            String_t label;
            size_t firstChild;
            size_t nextSibling;
            Element(int32_t key, const String_t& label)
                : key(key), label(label), firstChild(nil), nextSibling(nil)
                { }
        };

        std::vector<Element> m_data;
    };
}

#endif
