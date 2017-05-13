/**
  *  \file server/interface/talkfolderclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_TALKFOLDERCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKFOLDERCLIENT_HPP

#include "server/interface/talkfolder.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class TalkFolderClient : public TalkFolder {
     public:
        explicit TalkFolderClient(afl::net::CommandHandler& commandHandler);
        ~TalkFolderClient();

        virtual void getFolders(afl::data::IntegerList_t& result);
        virtual Info getInfo(int32_t ufid);
        virtual void getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results);
        virtual int32_t create(String_t name, afl::base::Memory<const String_t> args);
        virtual bool remove(int32_t ufid);
        virtual void configure(int32_t ufid, afl::base::Memory<const String_t> args);
        virtual afl::data::Value* getPMs(int32_t ufid, const ListParameters& params);

        static Info unpackInfo(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
