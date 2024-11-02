/**
  *  \file server/file/directorywrapper.hpp
  *  \brief Class server::file::DirectoryWrapper
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYWRAPPER_HPP
#define C2NG_SERVER_FILE_DIRECTORYWRAPPER_HPP

#include "afl/io/directory.hpp"
#include "util/serverdirectory.hpp"

namespace server { namespace file {

    class DirectoryItem;

    /** Wrap a DirectoryItem into a afl::io::Directory (read-only).
        Use this to call code that needs a Directory when you have a DirectoryItem.
        This implements read-only access, and does not attempt to meaningfully handle parallel modifications to the file space.
        It accesses the managed file space (hence, requires a DirectoryItem),
        but assumes reading and access checking to have been performed before.

        DirectoryWrapper only allows access to files in the directory, not to subdirectories
        (which might have different access permissions).

        DirectoryWrapper now implements util::ServerDirectory::Transport, not afl::io::Directory.
        Users only use the create() function. */
    class DirectoryWrapper : public util::ServerDirectory::Transport {
     public:
        /** Create Directory object wrapping the given item.
            @param item Item. Lifetime must exceed that of the resulting object.
            @return Directory */
        static afl::base::Ref<afl::io::Directory> create(DirectoryItem& item);

        // Transport:
        virtual void getFile(String_t name, afl::base::GrowableBytes_t& data);
        virtual void putFile(String_t name, afl::base::ConstBytes_t data);
        virtual void eraseFile(String_t name);
        virtual void getContent(std::vector<util::ServerDirectory::FileInfo>& result);
        virtual bool isValidFileName(String_t name) const;
        virtual bool isWritable() const;

     private:
        DirectoryWrapper(DirectoryItem& item);

        DirectoryItem& m_item;
    };

} }

#endif
