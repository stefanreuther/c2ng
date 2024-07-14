/**
  *  \file server/talk/talkrender.hpp
  *  \brief Class server::talk::TalkRender
  */
#ifndef C2NG_SERVER_TALK_TALKRENDER_HPP
#define C2NG_SERVER_TALK_TALKRENDER_HPP

#include "server/interface/talkrender.hpp"
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk {

    /** Implementation of RENDER commands. */
    class TalkRender : public server::interface::TalkRender {
     public:
        /** Constructor.
            @param session Session
            @param root Service root */
        TalkRender(Session& session, Root& root);

        virtual void setOptions(const Options& opts);
        virtual String_t render(const String_t& text, const Options& opts);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
