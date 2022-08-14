/**
  *  \file server/host/file/item.hpp
  *  \brief Base class server::host::file::Item
  */
#ifndef C2NG_SERVER_HOST_FILE_ITEM_HPP
#define C2NG_SERVER_HOST_FILE_ITEM_HPP

#include "afl/base/deletable.hpp"
#include "afl/container/ptrvector.hpp"
#include "server/interface/hostfile.hpp"

namespace server { namespace host { namespace file {

    /** Base class for an item in c2host's virtual filespace.

        c2host allows files to be listed and read using the HostFile interface.
        The Item hierarchy provides a simple interface to build the virtual file hierarchy.

        Item instances are short-lived and thus need not deal with cache invalidation or permission changes.
        Access checking/limiting is done by the Item classes, there is no separate access checking pass.
        Because items are short-lived, a class for a nested folder can assume all preconditions for the
        parent folder to be present (e.g. a game exists and is accessible), and need not check them again. */
    class Item : public afl::base::Deletable {
     public:
        typedef server::interface::HostFile::Info Info_t;
        typedef afl::container::PtrVector<Item> ItemVector_t;

        /** Get name.
            \return name (plain name, withoug path; same as getInfo().name, but more efficient) */
        virtual String_t getName() = 0;

        /** Get full information.
            \return information. Optional fields need not be present if this is the child of an entry that fills them in. */
        virtual Info_t getInfo() = 0;

        /** Find item by name.
            If an item is returned by listContent(), it needs to be found by this function.
            However, this function may also find functions not returned by listContent().

            \param name Name (plain name, without path)
            \return newly-allocated item, or null if not found
            \throw std::runtime_error 405 (Not a directory)
            \throw std::runtime_error 401 (Permission denied) */
        virtual Item* find(const String_t& name) = 0;

        /** Get content of directory.
            Fails for files.
            \param out [out] Result vector
            \throw std::runtime_error 405 (Not a directory)
            \throw std::runtime_error 401 (Permission denied) */
        virtual void listContent(ItemVector_t& out) = 0;

        /** Get content of file.
            Fails for directories.
            \return file content
            \throw std::runtime_error 401 (Permission denied) */
        virtual String_t getContent() = 0;

        /** Resolve path.
            Given a path with possible path separators ("/"), splits it into components and looks up each in turn.
            Empty path components are not allowed, ruling out paths starting or ending with "/", or having "//" in them.
            \param pathName Name
            \param out [out] Result
            \return final item
            \throw std::runtime_error 405 (Not a directory)
            \throw std::runtime_error 404 (Not found)
            \throw std::runtime_error 401 (Permission denied) */
        Item& resolvePath(const String_t& pathName, ItemVector_t& out);

     protected:
        /*
         *  Default Implementations
         */

        /** Possible default implementation of find(): use listContent().
            This lists the directory content using listContent() and looks for a matching item.
            \param name Name
            \return found item, or null */
        Item* defaultFind(const String_t& name);

        /** Possible default implementation of listContent(): fail.
            \param out [out] Result vector
            \throw std::runtime_error 405 (Not a directory) */
        void defaultList(ItemVector_t& out);

        /** Possible default implementation of getContent(): fail.
            \throw std::runtime_error 401 (Permission denied) */
        String_t defaultGetContent();
    };

} } }

#endif
