/**
  *  \file server/talk/group.hpp
  */
#ifndef C2NG_SERVER_TALK_GROUP_HPP
#define C2NG_SERVER_TALK_GROUP_HPP

#include "afl/net/redis/subtree.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/stringsetkey.hpp"
#include "server/interface/talkgroup.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/root.hpp"

namespace server { namespace talk {

    class Group {
     public:
        Group(Root& root, String_t groupId);

        afl::net::redis::HashKey      header();
        afl::net::redis::StringField  name();
        afl::net::redis::StringField  description();
        afl::net::redis::StringField  key();
        afl::net::redis::IntegerField unlisted();
        bool                          exists();

        afl::net::redis::IntegerSetKey forums();
        afl::net::redis::StringSetKey subgroups();

        String_t getParent();
        void setParent(String_t newParent, Root& root);

        server::interface::TalkGroup::Description describe(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root);

     private:
        afl::net::redis::Subtree m_group;
        String_t m_id;
    };

} }

#endif
