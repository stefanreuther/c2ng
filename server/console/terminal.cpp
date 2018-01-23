/**
  *  \file server/console/terminal.cpp
  *  \brief Interface server::console::Terminal
  */

#include "server/console/terminal.hpp"

// Convert a ContextStack_t into a string to use as a prompt.
String_t
server::console::Terminal::packContextStack(const ContextStack_t& st)
{
    String_t result;
    for (size_t i = 0, n = st.size(); i < n; ++i) {
        if (st[i]) {
            if (!result.empty()) {
                result += ' ';
            }
            result += st[i]->getName();
        }
    }
    return result;
}
