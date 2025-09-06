/**
  *  \file server/talk/talkgroup.cpp
  *  \brief Class server::talk::TalkGroup
  */

#include "server/talk/talkgroup.hpp"
#include "server/talk/group.hpp"
#include "server/talk/forum.hpp"
#include "afl/net/redis/sortoperation.hpp"
#include "server/errors.hpp"

namespace {
    void configureGroup(server::talk::Root& root, server::talk::Group& g, const server::interface::TalkGroup::Description& info)
    {
        if (const String_t* p = info.name.get()) {
            g.name().set(*p);
        }
        if (const String_t* p = info.description.get()) {
            g.description().set(*p);
        }
        if (const String_t* p = info.parentGroup.get()) {
            g.setParent(*p, root);
        }
        if (const String_t* p = info.key.get()) {
            g.key().set(*p);
        }
        if (const bool* p = info.unlisted.get()) {
            g.unlisted().set(*p);
        }
    }
}

server::talk::TalkGroup::TalkGroup(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::talk::TalkGroup::add(String_t groupId, const Description& info)
{
    // ex planetscentral/talk/cmdgroup.cc:doGroupAdd

    // Must be admin
    m_session.checkAdmin();

    // Verify
    Group group(m_root, groupId);
    if (group.exists()) {
        throw std::runtime_error(ALREADY_EXISTS);
    }
    configureGroup(m_root, group, info);
}

void
server::talk::TalkGroup::set(String_t groupId, const Description& info)
{
    // ex planetscentral/talk/cmdgroup.cc:doGroupSet
    m_session.checkAdmin();

    Group group(m_root, groupId);
    if (!group.exists()) {
        throw std::runtime_error(GROUP_NOT_FOUND);
    }
    configureGroup(m_root, group, info);
}

String_t
server::talk::TalkGroup::getField(String_t groupId, String_t fieldName)
{
    // ex planetscentral/talk/cmdgroup.cc:doGroupGet
    return Group(m_root, groupId).header().stringField(fieldName).get();
}

void
server::talk::TalkGroup::list(String_t groupId, afl::data::StringList_t& groups, afl::data::IntegerList_t& forums)
{
    // ex planetscentral/talk/cmdgroup.cc:doGroupList
    Group group(m_root, groupId);
    if (m_session.isAdmin() || !group.unlisted().get()) {
        group.subgroups()
            .sort()
            .sortLexicographical()
            .by(Group(m_root, "*").key())
            .getResult(groups);
        group.forums()
            .sort()
            .sortLexicographical()
            .by(m_root.forumRoot().subtree("*").hashKey("header").field("key"))
            .getResult(forums);
    }
}

server::talk::TalkGroup::Description
server::talk::TalkGroup::getDescription(String_t groupId)
{
    return Group(m_root, groupId).describe(server::talk::render::Context(m_root, m_session.getUser()), m_session.renderOptions(), m_root);
}

void
server::talk::TalkGroup::getDescriptions(const afl::data::StringList_t& groups, afl::container::PtrVector<Description>& results)
{
    for (size_t i = 0, n = groups.size(); i < n; ++i) {
        results.pushBackNew(new Description(getDescription(groups[i])));
    }
}

