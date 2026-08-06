/**
  *  \file server/file/fileshareimpl.hpp
  *  \brief Class server::file::FileShareImpl
  */
#ifndef C2NG_SERVER_FILE_FILESHAREIMPL_HPP
#define C2NG_SERVER_FILE_FILESHAREIMPL_HPP

#include "server/interface/fileshare.hpp"

namespace server { namespace file {

    class Session;
    class Root;

    /** Implementation of FileShare interface for c2file server. */
    class FileShareImpl : public server::interface::FileShare {
     public:
        /** Constructor.
            @param session Session object (provides user context)
            @param root Root (provides file space, logging, config) */
        FileShareImpl(Session& session, Root& root);

        // Interface operations:
        virtual int32_t getSequenceNumber(String_t userId);
        virtual void listDirectories(String_t userId, afl::container::PtrVector<Info>& result);
        virtual void listGameInfo(String_t userId, afl::container::PtrVector<server::interface::FileGame::GameInfo>& result);

     private:
        Session& m_session;
        Root& m_root;

        void checkUserId(const String_t& userId) const;
    };

} }

#endif
