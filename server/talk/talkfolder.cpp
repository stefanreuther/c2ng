/**
  *  \file server/talk/talkfolder.cpp
  *  \brief Class server::talk::TalkFolder
  */

#include <memory>
#include <stdexcept>
#include "server/talk/talkfolder.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
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

// Somehow, the ancient Win32 gcc fails to provide this...
#ifndef INT32_MAX
# define INT32_MAX 0x7FFFFFFF
#endif

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
server::talk::TalkFolder::getPMs(int32_t ufid, const ListParameters& params, const FilterParameters& filter)
{
    // ex doFolderListPMs
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder folder(u, ufid);
    folder.checkExistance(m_root);

    return executeListOperation(params, filter, folder.messages(), UserPM::PMSorter(m_root));
}

afl::data::Value*
server::talk::TalkFolder::executeListOperation(const ListParameters& params, const FilterParameters& filter, afl::net::redis::IntegerSetKey key, const Sorter& sorter)
{
    if (params.mode == ListParameters::WantMemberCheck) {
        // Member check: check individual value
        bool ok = key.contains(params.item);
        if (ok && filter.hasFlags()) {
            ok = (UserPM(m_root, params.item).flags(m_session.getUser()).get() & filter.flagMask) == filter.flagCheck;
        }
        return makeIntegerValue(ok);
    } else {
        // List operation
        bool hasFilter = false;
        afl::net::redis::SortOperation op(key.sort());
        op.get();
        if (filter.hasFlags()) {
            hasFilter = true;
            op.get(UserPM(m_root, Wildcard()).flags(m_session.getUser()));
        }
        if (const String_t* sortKey = params.sortKey.get()) {
            sorter.applySortKey(op, *sortKey);
        }

        if (!hasFilter) {
            // No filter: work entirely server-side
            switch (params.mode) {
             case ListParameters::WantRange:
                op.limit(params.start, params.count);
                return op.getResult();
             case ListParameters::WantAll:
                return op.getResult();
             case ListParameters::WantSize:
                return makeIntegerValue(key.size());
             case ListParameters::WantMemberCheck:
                // Handled above
                break;
            }
            return 0;
        } else {
            // Local filter operation
            std::auto_ptr<afl::data::Value> p(op.getResult());
            afl::data::Access a(p.get());
            size_t index = 0, limit = a.getArraySize();
            switch (params.mode) {
             case ListParameters::WantRange:
             case ListParameters::WantAll: {
                int32_t toSkip = (params.mode == ListParameters::WantAll ? 0         : params.start);
                int32_t toCopy = (params.mode == ListParameters::WantAll ? INT32_MAX : params.count);
                afl::data::Vector::Ref_t vv(afl::data::Vector::create());
                while (index < limit && toCopy > 0) {
                    int32_t pmId = a[index++].toInteger();
                    if (matchFilter(a, pmId, filter, index)) {
                        if (toSkip > 0) {
                            --toSkip;
                        } else {
                            vv->pushBackInteger(pmId);
                            --toCopy;
                        }
                    }
                }
                return new afl::data::VectorValue(vv);
             }
             case ListParameters::WantSize: {
                int32_t n = 0;
                while (index < limit) {
                    int32_t pmId = a[index++].toInteger();
                    if (matchFilter(a, pmId, filter, index)) {
                        ++n;
                    }
                }
                return makeIntegerValue(n);
             }
             case ListParameters::WantMemberCheck:
                // Handled above
                break;
            }
            return 0;
        }
    }
}

bool
server::talk::TalkFolder::matchFilter(afl::data::Access a, int32_t pmId, const FilterParameters& filter, size_t& index)
{
    UserPM pm(m_root, pmId);
    if (filter.hasFlags()) {
        int32_t flagValue = a[index++].toInteger();
        if ((flagValue & filter.flagMask) != filter.flagCheck) {
            return false;
        }
    }
    return true;
}
