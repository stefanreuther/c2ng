/**
  *  \file server/interface/fileshare.hpp
  *  \brief Interface server::interface::FileShare
  */
#ifndef C2NG_SERVER_INTERFACE_FILESHARE_HPP
#define C2NG_SERVER_INTERFACE_FILESHARE_HPP

#include "afl/base/deletable.hpp"
#include "server/interface/filegame.hpp"
#include "afl/container/ptrvector.hpp"

namespace server { namespace interface {

    /** Interface for accessing shared file/folder information. */
    class FileShare : public afl::base::Deletable {
     public:
        /** Information about a shared file path. */
        struct Info {
            String_t pathName;   ///< Path name ("u/foo/bar").
            String_t owner;      ///< Owner of file share ("1003").
            int seqNr;           ///< Sequence number.

            /** Constructor. Makes a blank info. */
            Info()
                : pathName(), owner(), seqNr(0)
                { }
        };

        /** Get sequence number (SHARESEQ).
            If this number changes from one call to the next,
            this means that new file shares were added.
            @param userId User Id. */
        virtual int32_t getSequenceNumber(String_t userId) = 0;

        /** List directories shared with a user (SHARELS).
            @param userId [in]  User Id.
            @param result [out] Result information is placed here. */
        virtual void listDirectories(String_t userId, afl::container::PtrVector<Info>& result) = 0;

        /** List game information for shared files.
            @param userId [in]  User Id.
            @param result [out] Restul information is placed here. */
        virtual void listGameInfo(String_t userId, afl::container::PtrVector<FileGame::GameInfo>& result) = 0;
    };

} }

#endif
