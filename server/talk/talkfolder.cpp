/**
  *  \file server/talk/talkfolder.cpp
  *  \brief Class server::talk::TalkFolder
  */

#include <memory>
#include <stdexcept>
#include "server/talk/talkfolder.hpp"
#include "afl/net/redis/integersetoperation.hpp"
#include "afl/net/redis/stringfield.hpp"
#include "server/errors.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/sorter.hpp"
#include "server/talk/talkforum.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/userpm.hpp"

namespace {
    void configureFolder(server::talk::UserFolder& folder, afl::base::Memory<const String_t> args)
    {
        while (const String_t* pKey = args.eat()) {
            const String_t* pValue = args.eat();
            if (pValue == 0) {
                throw std::runtime_error(server::INVALID_NUMBER_OF_ARGUMENTS);
            }
            folder.header().stringField(*pKey).set(*pValue);
        }
    }

}

server::talk::TalkFolder::TalkFolder(Session& session, Root& root)
    : m_session(session),
      m_root(root)
{ }

void
server::talk::TalkFolder::getFolders(afl::data::IntegerList_t& result)
{
    // ex doFolderList
    m_session.checkUser();

    UserFolder::defaultFolders(m_root).
        merge(User(m_root, m_session.getUser()).pmFolders()).
        getAll(result);
}

server::interface::TalkFolder::Info
server::talk::TalkFolder::getInfo(int32_t ufid)
{
    // ex doFolderStat
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder folder(u, ufid);
    return folder.describe(folder.checkExistance(m_root), m_root);
}

void
server::talk::TalkFolder::getInfo(afl::base::Memory<const int32_t> ufids, afl::container::PtrVector<Info>& results)
{
    // ex doFolderMStat
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    while (const int32_t* p = ufids.eat()) {
        std::auto_ptr<Info> tmp;
        try {
            UserFolder folder(u, *p);
            tmp.reset(new Info(folder.describe(folder.checkExistance(m_root), m_root)));
        }
        catch (...) { }
        results.pushBackNew(tmp.release());
    }
}

int32_t
server::talk::TalkFolder::create(String_t name, afl::base::Memory<const String_t> args)
{
    m_session.checkUser();

    // Create folder
    User u(m_root, m_session.getUser());
    int32_t newUfid = UserFolder::allocateFolder(u);
    UserFolder folder(u, newUfid);
    u.pmFolders().add(newUfid);

    // Configure the folder
    folder.header().stringField("name").set(name);
    configureFolder(folder, args);

    return newUfid;
}

bool
server::talk::TalkFolder::remove(int32_t ufid)
{
    // ex doFolderRemove
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder folder(u, ufid);

    // Try to remove it.
    if (!u.pmFolders().remove(ufid)) {
        // Remove failed, this is not a user folder. Either it did not exist,
        // or it is a global default folder.
        return false;
    } else {
        // Unlink all messages
        afl::data::IntegerList_t msgs;
        folder.messages().getAll(msgs);
        for (size_t i = 0, n = msgs.size(); i != n; ++i) {
            UserPM(m_root, msgs[i]).removeReference();
        }

        // Remove the folder
        folder.remove();
        return true;
    }
}

void
server::talk::TalkFolder::configure(int32_t ufid, afl::base::Memory<const String_t> args)
{
    // ex doFolderSet
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    UserFolder folder(u, ufid);
    folder.checkExistance(m_root);
    configureFolder(folder, args);
}

afl::data::Value*
server::talk::TalkFolder::getPMs(int32_t ufid, const ListParameters& params)
{
    // ex doFolderListPMs
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder folder(u, ufid);
    folder.checkExistance(m_root);

    return TalkForum::executeListOperation(params, folder.messages(), UserPM::PMSorter(m_root));
}
