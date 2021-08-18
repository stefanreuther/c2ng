/**
  *  \file server/play/fs/directory.hpp
  *  \brief Class server::play::fs::Directory
  */
#ifndef C2NG_SERVER_PLAY_FS_DIRECTORY_HPP
#define C2NG_SERVER_PLAY_FS_DIRECTORY_HPP

#include "afl/io/directory.hpp"
#include "server/play/fs/session.hpp"

namespace server { namespace play { namespace fs {

    /** Directory on file server. */
    class Directory : public afl::io::Directory {
     public:
        class Stream;
        class Enum;
        class Entry;

        /** Create directory.
            \param session Session (file server connection)
            \param dirName Directory name */
        static afl::base::Ref<Directory> create(afl::base::Ref<Session> session, String_t dirName);

        // Directory virtuals:
        virtual afl::base::Ref<afl::io::DirectoryEntry> getDirectoryEntryByName(String_t name);
        virtual afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > getDirectoryEntries();
        virtual afl::base::Ptr<afl::io::Directory> getParentDirectory();
        virtual String_t getDirectoryName();
        virtual String_t getTitle();

     private:
        Directory(const afl::base::Ref<Session>& session, const String_t& dirName);

        afl::base::Ref<Session> m_session;
        String_t m_dirName;

        String_t makePathName(const String_t& fileName) const;
    };

} } }

#endif
