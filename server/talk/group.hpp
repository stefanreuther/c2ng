/**
  *  \file server/talk/group.hpp
  *  \brief Class server::talk::Group
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

    /** Group.
        Represents access to a forum group definition.
        A group contains
        - metainformation
        - a list of forums
        - a list of subgroups

        A group is identified by a group Id, a string.
        To bootstrap, services use a root group name.
        (There is no way to obtain a list of root groups.) */
    class Group {
     public:
        /** Constructor.
            @param root   Service root
            @param groupId Group Id */
        Group(Root& root, String_t groupId);

        /*
         *  Database access
         */

        /** Access header.
            @return header */
        afl::net::redis::HashKey header();

        /** Access group name.
            Format: string
            @return group name */
        afl::net::redis::StringField name();

        /** Access group description.
            Format: a string suitable for rendering (i.e. type prefix + text).
            @return group description field. */
        afl::net::redis::StringField description();

        /** Access sort key.
            This is used to sort groups.
            @return sort key field */
        afl::net::redis::StringField key();

        /** Access "unlisted" flag.
            @return flag */
        afl::net::redis::IntegerField unlisted();

        /** Check existence of this group.
            A group exists if its header exists in the database.
            @return true if group exists */
        bool exists();

        /** Access forums.
            @return set of all forums */
        afl::net::redis::IntegerSetKey forums();

        /** Access subgroups.
            @return set of all subgroups */
        afl::net::redis::StringSetKey subgroups();

        /** Get parent.
            @return parent group name; empty if none */
        String_t getParent();

        /** Set parent.
            @param newParent  New parent group name; empty to make this a root group
            @param root       Root; to access other groups */
        void setParent(String_t newParent, Root& root);

        /** Describe this group.
            @param ctx     Rendering context
            @param opts    Rendering options
            @param root    Root
            @return description suitable for TalkGroup */
        server::interface::TalkGroup::Description describe(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root);

     private:
        afl::net::redis::Subtree m_group;
        String_t m_id;
    };

} }

#endif
