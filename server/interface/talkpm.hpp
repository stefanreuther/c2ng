/**
  *  \file server/interface/talkpm.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPM_HPP
#define C2NG_SERVER_INTERFACE_TALKPM_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/base/optional.hpp"
#include "server/interface/talkrender.hpp"
#include "server/types.hpp"
#include "afl/container/ptrvector.hpp"

namespace server { namespace interface {

    class TalkPM : public afl::base::Deletable {
     public:
        static const int32_t PMStateRead      = 1;
        static const int32_t PMStateReplied   = 2;
        static const int32_t PMStateForwarded = 4;

        typedef TalkRender::Options Options;

        struct Info {
            String_t author;
            String_t receivers;
            Time_t time;
            String_t subject;
            int32_t flags;
            afl::base::Optional<int32_t> parent;
        };

        virtual int32_t create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent) = 0;
        virtual Info getInfo(int32_t folder, int32_t pmid) = 0;
        virtual void getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results) = 0;
        virtual int32_t copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids) = 0;
        virtual int32_t move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids) = 0;
        virtual int32_t remove(int32_t folder, afl::base::Memory<const int32_t> pmids) = 0;
        virtual String_t render(int32_t folder, int32_t pmid, const Options& options) = 0;
        virtual void render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result) = 0;
        virtual int32_t changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids) = 0;
    };

} }

#endif
