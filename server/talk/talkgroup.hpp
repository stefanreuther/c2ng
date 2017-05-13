/**
  *  \file server/talk/talkgroup.hpp
  *  \brief Class server::talk::TalkGroup
  */
#ifndef C2NG_SERVER_TALK_TALKGROUP_HPP
#define C2NG_SERVER_TALK_TALKGROUP_HPP

#include "server/interface/talkgroup.hpp"
#include "server/talk/session.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk {

    /** Implementation of GROUP commands. */
    class TalkGroup : public server::interface::TalkGroup {
     public:
        /** Constructor.
            \param session Session
            \param root Service root */
        TalkGroup(Session& session, Root& root);

        // TalkGroup interface:
        virtual void add(String_t groupId, const Description& info);
        virtual void set(String_t groupId, const Description& info);
        virtual String_t getField(String_t groupId, String_t fieldName);
        virtual void list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums);
        virtual Description getDescription(String_t groupId);
        virtual void getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results);

     private:
        Session& m_session;
        Root& m_root;
    };

} }

#endif
