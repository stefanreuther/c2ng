/**
  *  \file server/host/file/item.cpp
  *  \brief Base class server::host::file::Item
  */

#include <stdexcept>
#include "server/host/file/item.hpp"
#include "server/errors.hpp"

server::host::file::Item&
server::host::file::Item::resolvePath(const String_t& pathName, ItemVector_t& out)
{
    String_t::size_type pos = 0;
    String_t::size_type next;
    Item* origin = this;
    while ((next = pathName.find('/', pos)) != String_t::npos) {
        // Refuse empty name straightaway.
        // This also refuses names starting with slash, double-slash, etc.
        if (next == pos) {
            throw std::runtime_error(FILE_NOT_FOUND);
        }

        Item* p = origin->find(pathName.substr(pos, next-pos));
        if (!p) {
            throw std::runtime_error(FILE_NOT_FOUND);
        }

        // Advance
        out.pushBackNew(p);
        origin = p;
        pos = next+1;
    }

    // Final component
    // This check will refuse the empty file name.
    if (pos == pathName.size()) {
        throw std::runtime_error(FILE_NOT_FOUND);
    }

    Item* p = origin->find(pathName.substr(pos));
    if (!p) {
        throw std::runtime_error(FILE_NOT_FOUND);
    }
    out.pushBackNew(p);

    return *p;
}

server::host::file::Item*
server::host::file::Item::defaultFind(const String_t& name)
{
    ItemVector_t v;
    listContent(v);
    for (size_t i = 0, n = v.size(); i < n; ++i) {
        if (Item* p = v[i]) {
            if (p->getName() == name) {
                return v.extractElement(i);
            }
        }
    }
    return 0;
}

void
server::host::file::Item::defaultList(ItemVector_t& /*out*/)
{
    throw std::runtime_error(NOT_A_DIRECTORY);
}

String_t
server::host::file::Item::defaultGetContent()
{
    // PERMISSION_DENIED is used by server::file::PathResolver::resolveToFile when trying to read a directory.
    throw std::runtime_error(PERMISSION_DENIED);
}
