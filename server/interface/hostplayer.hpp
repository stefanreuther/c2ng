/**
  *  \file server/interface/hostplayer.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTPLAYER_HPP
#define C2NG_SERVER_INTERFACE_HOSTPLAYER_HPP

#include <map>
#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"

namespace server { namespace interface {

    class HostPlayer : public afl::base::Deletable {
     public:
        struct Info {
            String_t longName;
            String_t shortName;
            String_t adjectiveName;
            afl::data::StringList_t userIds;
            int32_t numEditable;
            bool joinable;

            Info();
        };

        enum FileStatus {
            Stale,              // directory is stale, file upload allowed
            Allow,              // file upload allowed, use FileBase.putFile
            Turn,               // turn file, use HostTurn.submit
            Refuse              // file upload not allowed
        };

        // PLAYERJOIN game:GID slot:Int user:UID
        virtual void join(int32_t gameId, int32_t slot, String_t userId) = 0;

        // PLAYERSUBST game:GID slot:Int user:UID
        virtual void substitute(int32_t gameId, int32_t slot, String_t userId) = 0;

        // PLAYERRESIGN game:GID slot:Int user:UID
        virtual void resign(int32_t gameId, int32_t slot, String_t userId) = 0;

        // PLAYERADD game:GID user:UID
        virtual void add(int32_t gameId, String_t userId) = 0;

        // PLAYERLS game:GID [ALL]
        virtual void list(int32_t gameId, bool all, std::map<int,Info>& result) = 0;

        // PLAYERSTAT game:GID slot:Int
        virtual Info getInfo(int32_t gameId, int32_t slot) = 0;

        // PLAYERSETDIR game:GID user:UID dir:FileName
        virtual void setDirectory(int32_t gameId, String_t userId, String_t dirName) = 0;

        // PLAYERGETDIR game:GID user:UID
        virtual String_t getDirectory(int32_t gameId, String_t userId) = 0;

        // PLAYERCHECKFILE game:GID user:UID name:Str [DIR dir:FileName]
        virtual FileStatus checkFile(int32_t gameId, String_t userId, String_t fileName, afl::base::Optional<String_t> dirName) = 0;

        static String_t formatFileStatus(FileStatus st);
        static bool parseFileStatus(const String_t& str, FileStatus& st);
    };

} }

#endif
