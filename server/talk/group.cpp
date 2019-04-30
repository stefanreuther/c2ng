/**
  *  \file server/talk/group.cpp
  */

#include "server/talk/group.hpp"
#include "server/talk/render/render.hpp"

// group:$GRID:header : hash
//         - name                          Name
//         - description                   Description
//         - parent                        If present, GRID of parent group
//         - key                           Sort key for displaying groups
                             
//         Well-known GRIDs:    
//         - root               
//         - active                        Active/joining games
//         - finished                      Finished games

// group:$GRID:groups : set
// group:$GRID:forums : set
//         - GRIDs of subgroups
//         - FIDs of forums

server::talk::Group::Group(Root& root, String_t groupId)
    : m_group(root.groupRoot().subtree(groupId)),
      m_id(groupId)
{
    // ex Group::Group
}

afl::net::redis::HashKey
server::talk::Group::header()
{
    // ex Group::header
    return m_group.hashKey("header");
}

afl::net::redis::StringField
server::talk::Group::name()
{
    // ex Group::name
    return header().stringField("name");
}

afl::net::redis::StringField
server::talk::Group::description()
{
    // ex Group::description
    return header().stringField("description");
}

afl::net::redis::StringField
server::talk::Group::key()
{
    // ex Group::key
    return header().stringField("key");
}

afl::net::redis::IntegerField
server::talk::Group::unlisted()
{
    // ex Group::unlisted
    return header().intField("unlisted");
}

bool
server::talk::Group::exists()
{
    // ex Group::exists
    return header().exists();
}

afl::net::redis::IntegerSetKey
server::talk::Group::forums()
{
    // ex Group::forums
    return m_group.intSetKey("forums");
}

afl::net::redis::StringSetKey
server::talk::Group::subgroups()
{
    // ex Group::subgroups
    return m_group.stringSetKey("groups");
}

String_t
server::talk::Group::getParent()
{
    // ex Group::getParent
    return header().stringField("parent").get();
}

void
server::talk::Group::setParent(String_t newParent, Root& root)
{
    // ex Group::setParent
    String_t oldParent = getParent();
    if (oldParent != newParent) {
        Group oldGroup(root, oldParent);
        Group newGroup(root, newParent);
        if (oldParent.empty()) {
            newGroup.subgroups().add(m_id);
        } else if (newParent.empty()) {
            oldGroup.subgroups().remove(m_id);
        } else {
            oldGroup.subgroups().moveTo(m_id, newGroup.subgroups());
        }
        header().stringField("parent").set(newParent);
    }
}

server::interface::TalkGroup::Description
server::talk::Group::describe(const server::talk::render::Context& ctx, const server::talk::render::Options& opts, Root& root)
{
    // ex Group::describe

    /* @type TalkGroupInfo
       Information about a forum group.

       @key name:Str         (Name)
       @key description:Str  (Description, rendered using {RENDEROPTION})
       @key parent:GRID      (Parent group)
       @key unlisted:Int     (If nonzero, group is unlistable) */

    // FIXME: can we use HMGET?
    // FIXME: this traditionally does not report "key" although it could now.
    server::interface::TalkGroup::Description result;
    result.name = name().get();
    result.description = server::talk::render::renderText(description().get(), ctx, opts, root);
    result.parentGroup = getParent();
    result.unlisted = unlisted().get() != 0;
    return result;
}
