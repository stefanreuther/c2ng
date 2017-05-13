/**
  *  \file server/interface/talkpmclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKPMCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKPMCLIENT_HPP

#include "server/interface/talkpm.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class TalkPMClient : public TalkPM {
     public:
        TalkPMClient(afl::net::CommandHandler& commandHandler);
        ~TalkPMClient();

        virtual int32_t create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent);
        virtual Info getInfo(int32_t folder, int32_t pmid);
        virtual void getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results);
        virtual int32_t copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids);
        virtual int32_t move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids);
        virtual int32_t remove(int32_t folder, afl::base::Memory<const int32_t> pmids);
        virtual String_t render(int32_t folder, int32_t pmid, const Options& options);
        virtual void render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result);
        virtual int32_t changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids);

        static Info unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
