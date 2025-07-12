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
    class Directory {
     public:
        class Transport;

        /** Create directory.
            \param session Session (file server connection)
            \param dirName Directory name */
        static afl::base::Ref<afl::io::Directory> create(afl::base::Ref<Session> session, String_t dirName);
    };

} } }

#endif
