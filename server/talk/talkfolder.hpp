/**
  *  \file server/talk/talkfolder.hpp
  *  \brief Class server::talk::TalkFolder
  */
#ifndef C2NG_SERVER_TALK_TALKFOLDER_HPP
#define C2NG_SERVER_TALK_TALKFOLDER_HPP

#include "server/interface/talkfolder.hpp"

namespace server { namespace talk {

    class Session;
    class Root;

    /** Implementation of FOLDER commands. */
    class TalkFolder : public server::interface::TalkFolder {
     public:
        /** Constructor.
            \param session Session
            \param root Service root */
        TalkFolder(Session& session, Root& root);

        // TalkFolder:
        virtual void getFolders(afl::data::IntegerList_t& result);
        virtual Info getInfo(int32_t ufid);
        virtual void getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results);
        virtual int32_t create(String_t name, afl::base::Memory<const String_t> args);
        virtual bool remove(int32_t ufid);
        virtual void configure(int32_t ufid, afl::base::Memory<const String_t> args);
        virtual afl::data::Value* getPMs(int32_t ufid, const ListParameters& params);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
