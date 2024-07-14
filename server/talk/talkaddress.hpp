/**
  *  \file server/talk/talkaddress.hpp
  *  \brief Class server::talk::TalkAddress
  */
#ifndef C2NG_SERVER_TALK_TALKADDRESS_HPP
#define C2NG_SERVER_TALK_TALKADDRESS_HPP

#include "server/interface/talkaddress.hpp"

namespace server { namespace talk {

    class Session;
    class Root;
    class TextNode;

    /** Implementation of ADDR commands. */
    class TalkAddress : public server::interface::TalkAddress {
     public:
        /** Constructor.
            \param session Session
            \param root Root */
        TalkAddress(Session& session, Root& root);

        // TalkAddress:
        virtual void parse(afl::base::Memory<const String_t> in, afl::data::StringList_t& out);
        virtual void render(afl::base::Memory<const String_t> in, afl::data::StringList_t& out);

     private:
        Session& m_session;
        Root& m_root;

        String_t parseReceiver(const String_t& in);
        bool renderReceiver(const String_t& in, TextNode& out);
        String_t renderRawReceiver(const String_t& in);
    };

} }

#endif
