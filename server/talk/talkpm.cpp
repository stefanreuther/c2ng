/**
  *  \file server/talk/talkpm.cpp
  *  \brief Class server::talk::TalkPM
  */

#include <set>
#include <stdexcept>
#include "server/talk/talkpm.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/subtree.hpp"
#include "afl/string/parse.hpp"
#include "game/v3/structures.hpp"
#include "server/errors.hpp"
#include "server/talk/notifier.hpp"
#include "server/talk/ratelimit.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/root.hpp"
#include "server/talk/session.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userfolder.hpp"
#include "server/talk/userpm.hpp"

namespace {
    const int32_t PMSystemInboxFolder  = 1;
    const int32_t PMSystemOutboxFolder = 2;

    /** Parse a single receiver.
        \param in Receiver
        \param out Individual users will be reported here */
    // FIXME: move these into a separate module to make them testable
    // FIXME: use StringParser
    // FIXME: avoid reference to game/v3
    void parseReceiver(const String_t& in, std::set<String_t>& out, server::talk::Root& root)
    {
        if (in.size() > 2 && in.compare(0, 2, "u:", 2) == 0) {
            out.insert(String_t(in, 2, in.size()-2));
        } else if (in.size() > 2 && in.compare(0, 2, "g:", 2) == 0) {
            String_t::size_type divi = in.find(':', 2);
            if (divi == String_t::npos) {
                // Parse
                int32_t gameId;
                if (!afl::string::strToInteger(String_t(in, 2), gameId)) {
                    throw std::runtime_error(server::INVALID_RECEIVER);
                }

                // Validate
                afl::net::redis::Subtree g(root.gameRoot());
                if (!g.intSetKey("all").contains(gameId) || g.subtree(gameId).stringKey("state").get() == "deleted") {
                    throw std::runtime_error(server::INVALID_RECEIVER);
                }

                // Get
                afl::data::StringList_t result;
                g.subtree(gameId).hashKey("users").getAll(result);
                for (size_t i = 0; i < result.size(); i += 2) {
                    if (result[i+1] != "0") {
                        out.insert(result[i]);
                    }
                }
            } else {
                // Parse
                int32_t gameId = 0, slotId = 0;
                if (!afl::string::strToInteger(String_t(in, 2, divi-2), gameId)
                    || !afl::string::strToInteger(String_t(in, divi+1), slotId)
                    || slotId <= 0 || slotId > game::v3::structures::NUM_PLAYERS)
                {
                    throw std::runtime_error(server::INVALID_RECEIVER);
                }

                // Validate
                afl::net::redis::Subtree g(root.gameRoot());
                if (!g.intSetKey("all").contains(gameId) || g.subtree(gameId).stringKey("state").get() == "deleted") {
                    throw std::runtime_error(server::INVALID_RECEIVER);
                }

                // Get
                afl::data::StringList_t users;
                g.subtree(gameId).subtree("player").subtree(slotId).stringListKey("users").getAll(users);
                for (size_t i = 0; i < users.size(); ++i) {
                    out.insert(users[i]);
                }
            }
        } else {
            throw std::runtime_error(server::INVALID_RECEIVER);
        }
    }

    /** Parse a receiver list.
        \param in Receiver list as given in the command
        \param out Individual users will be reported here */
    void parseReceivers(const String_t& in, std::set<String_t>& out, server::talk::Root& root)
    {
        // This is a machine interface, so there's no need to normalize.
        // 'in' is expected to contain comma-separated receivers, and that's it.
        String_t::size_type a = 0;
        String_t::size_type b;
        while ((b = in.find(',', a)) != String_t::npos) {
            parseReceiver(String_t(in, a, b-a), out, root);
            a = b+1;
        }
        parseReceiver(String_t(in, a, in.size()-a), out, root);
    }
}

server::talk::TalkPM::TalkPM(Session& session, Root& root)
    : m_session(session), m_root(root)
{ }

server::talk::TalkPM::~TalkPM()
{ }

int32_t
server::talk::TalkPM::create(String_t receivers, String_t subject, String_t text, afl::base::Optional<int32_t> parent)
{
    // ex doPMNew
    m_session.checkUser();

    // PM permission?
    const String_t sender = m_session.getUser();
    User user(m_root, sender);
    if (!user.isAllowedToSendPMs()) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // Check receivers
    std::set<String_t> recv;
    parseReceivers(receivers, recv, m_root);
    if (recv.empty()) {
        throw std::runtime_error(NO_RECEIVERS);
    }

    // Rate limit
    const Time_t time = m_root.getTime();
    const Configuration& config = m_root.config();
    const int32_t cost = config.rateCostPerMail + config.rateCostPerReceiver * int32_t(recv.size());
    if (!checkRateLimit(cost, time, config, user, m_root.log())) {
        throw std::runtime_error(PERMISSION_DENIED);
    }

    // Create the message
    int32_t pmid = UserPM::allocatePM(m_root);
    UserPM pm(m_root, pmid);

    pm.author().set(sender);
    pm.receivers().set(receivers);
    pm.subject().set(subject);
    pm.time().set(time);
    pm.text().set(text);
    pm.flags(sender).set(PMStateRead);
    if (const int32_t* p = parent.get()) {
        pm.parentMessageId().set(*p);
    }

    // Distribute the message
    afl::data::StringList_t notifyIndividual;
    afl::data::StringList_t notifyGroup;
    if (UserFolder(user, PMSystemOutboxFolder).messages().add(pmid)) {
        pm.addReference();
    }

    for (std::set<String_t>::const_iterator i = recv.begin(); i != recv.end(); ++i) {
        User u(m_root, *i);
        UserFolder folder(u, PMSystemInboxFolder);
        if (folder.messages().add(pmid)) {
            pm.addReference();

            // Process notifications
            if (*i != sender) {
                String_t notify = u.getPMMailType();
                if (notify != "none") {
                    if (notify == "info") {
                        if (folder.unreadMessages().get() == 0) {
                            notifyGroup.push_back("user:" + *i);
                        }
                    } else {
                        notifyIndividual.push_back("user:" + *i);
                    }
                }
                folder.unreadMessages().set(true);
            }
        }
    }

    // Send notifications
    if (Notifier* p = m_root.getNotifier()) {
        p->notifyPM(pm, notifyIndividual, notifyGroup);
    }

    // Result is message Id. Might be useful.
    return pmid;
}

server::interface::TalkPM::Info
server::talk::TalkPM::getInfo(int32_t folder, int32_t pmid)
{
    // ex doPMStat
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    if (!UserFolder(u, folder).messages().contains(pmid)) {
        // No need to verify that the folder exists; if it does not exist,
        // it will report empty.
        throw std::runtime_error(PM_NOT_FOUND);
    } else {
        // Report the message
        return UserPM(m_root, pmid).describe(m_session.getUser(), folder);
    }
}

void
server::talk::TalkPM::getInfo(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<Info>& results)
{
    // ex doPMMStat
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    UserFolder uf(u, folder);
    while (const int32_t* p = pmids.eat()) {
        if (!uf.messages().contains(*p)) {
            results.pushBackNew(0);
        } else {
            results.pushBackNew(new Info(UserPM(m_root, *p).describe(m_session.getUser(), folder)));
        }
    }
}

int32_t
server::talk::TalkPM::copy(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
{
    // ex doPMCopy
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    UserFolder srcfolder(u, sourceFolder);
    UserFolder dstfolder(u, destFolder);

    // Verify that destination exists
    dstfolder.checkExistance(m_root);

    // Copy
    int32_t count = 0;
    while (const int32_t* p = pmids.eat()) {
        const int32_t pmid = *p;
        if (srcfolder.messages().contains(pmid)) {
            if (dstfolder.messages().add(pmid)) {
                UserPM(m_root, pmid).addReference();
            }
            ++count;
        }
    }
    return count;
}

int32_t
server::talk::TalkPM::move(int32_t sourceFolder, int32_t destFolder, afl::base::Memory<const int32_t> pmids)
{
    // ex doPMMove
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    UserFolder srcfolder(u, sourceFolder);
    UserFolder dstfolder(u, destFolder);

    // Verify that destination exists
    dstfolder.checkExistance(m_root);

    // Move
    int32_t count = 0;
    while (const int32_t* p = pmids.eat()) {
        const int32_t pmid = *p;
        if (srcfolder.messages().remove(pmid)) {
            if (!dstfolder.messages().add(pmid)) {
                // I removed it from the source, but cannot add it to
                // the destination, so that's one lost reference.
                UserPM(m_root, pmid).removeReference();
            }
            ++count;
        }
    }

    return count;
}

int32_t
server::talk::TalkPM::remove(int32_t folder, afl::base::Memory<const int32_t> pmids)
{
    // ex doPMRemove
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    UserFolder uf(u, folder);

    // Move
    int32_t count = 0;
    while (const int32_t* p = pmids.eat()) {
        int32_t pmid = *p;
        if (uf.messages().remove(pmid)) {
            UserPM(m_root, pmid).removeReference();
            ++count;
        }
    }

    return count;
}

String_t
server::talk::TalkPM::render(int32_t folder, int32_t pmid, const Options& options)
{
    // ex doPMRender
    m_session.checkUser();

    // Do it
    User u(m_root, m_session.getUser());
    if (!UserFolder(u, folder).messages().contains(pmid)) {
        // No need to verify that the folder exists; if it does not exist,
        // it will report empty.
        throw std::runtime_error(PM_NOT_FOUND);
    } else {
        // Report the message
        render::Context ctx(m_session.getUser());
        ctx.setMessageAuthor(UserPM(m_root, pmid).author().get());

        render::Options temporaryOptions(m_session.renderOptions());
        temporaryOptions.updateFrom(options);

        return render::renderText(UserPM(m_root, pmid).text().get(), ctx, temporaryOptions, m_root);
    }
}

void
server::talk::TalkPM::render(int32_t folder, afl::base::Memory<const int32_t> pmids, afl::container::PtrVector<String_t>& result)
{
    // ex doPMMRender
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder uf(u, folder);
    while (const int32_t* p = pmids.eat()) {
        const int32_t pmid = *p;
        if (!uf.messages().contains(pmid)) {
            result.pushBackNew(0);
        } else {
            // Render the message
            render::Context ctx(m_session.getUser());
            ctx.setMessageAuthor(UserPM(m_root, pmid).author().get());
            result.pushBackNew(new String_t(render::renderText(UserPM(m_root, pmid).text().get(), ctx, m_session.renderOptions(), m_root)));
        }
    }
}

int32_t
server::talk::TalkPM::changeFlags(int32_t folder, int32_t flagsToClear, int32_t flagsToSet, afl::base::Memory<const int32_t> pmids)
{
    // ex doPMFlag
    m_session.checkUser();

    User u(m_root, m_session.getUser());
    UserFolder uf(u, folder);

    int32_t result = 0;
    while (const int32_t* p = pmids.eat()) {
        const int32_t pmid = *p;
        if (uf.messages().contains(pmid)) {
            UserPM msg(m_root, pmid);
            msg.flags(m_session.getUser()).set((msg.flags(m_session.getUser()).get() & ~flagsToClear) | flagsToSet);
            ++result;
        }
    }

    return result;
}
