/**
  *  \file server/talk/notify.cpp
  *  \brief Notification Generation
  */

#include <algorithm>
#include "server/talk/notify.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/net/redis/stringsetoperation.hpp"
#include "afl/string/format.hpp"
#include "server/talk/forum.hpp"
#include "server/talk/message.hpp"
#include "server/talk/render/context.hpp"
#include "server/talk/render/options.hpp"
#include "server/talk/render/render.hpp"
#include "server/talk/root.hpp"
#include "server/talk/topic.hpp"
#include "server/talk/user.hpp"
#include "server/talk/userpm.hpp"

using afl::data::StringList_t;

void
server::talk::notifyMessage(Message& msg, Root& root, server::interface::MailQueue& mq)
{
    // ex planetscentral/talk/notify.h:notifyMessage
    // Obtain parents
    Topic topic(root, msg.topicId().get());
    Forum forum(root, topic.forumId().get());

    // Get sender
    const String_t author = msg.author().get();

    // -- Topic --
    // Get topic watchers
    StringList_t topicWatchers;
    topic.watchers().getAll(topicWatchers);
    std::sort(topicWatchers.begin(), topicWatchers.end());

    // Get post read permissions
    String_t readPermissions = topic.readPermissions().get();
    if (readPermissions.empty()) {
        readPermissions = forum.readPermissions().get();
    }

    // Find topic watchers that have not been notified and mark them.
    // The author of the posting will not be notified.
    StringList_t groupReceivers;
    StringList_t individualReceivers;
    for (StringList_t::size_type i = 0; i < topicWatchers.size(); ++i) {
        if (topicWatchers[i] != author && root.checkUserPermission(readPermissions, topicWatchers[i])) {
            User u(root, topicWatchers[i]);
            if (u.isWatchIndividual()) {
                individualReceivers.push_back("user:" + topicWatchers[i]);
            } else if (u.notifiedTopics().add(topic.getId())) {
                groupReceivers.push_back("user:" + topicWatchers[i]);
            } else {
                // No notification for you
            }
        }
    }

    // Send message. We do not use a uniquifier here, because a future
    // message will have different receivers.
    if (!groupReceivers.empty()) {
        mq.startMessage("talk-topic", afl::base::Nothing);
        mq.addParameter("forum", forum.name().get());
        mq.addParameter("subject", msg.subject().get());
        mq.addParameter("posturl", root.linkFormatter().makePostUrl(topic.getId(), topic.subject().get(), msg.getId()));
        mq.send(groupReceivers);
    }
    if (!individualReceivers.empty()) {
        server::talk::render::Context ctx(author);
        ctx.setMessageId(msg.getId());

        server::talk::render::Options opts;
        opts.setBaseUrl(root.config().baseUrl);
        opts.setFormat("mail");

        const String_t renderedText = server::talk::render::renderText(msg.text().get(), ctx, opts, root);

        mq.startMessage("talk-topic-message", afl::base::Nothing);
        mq.addParameter("forum", forum.name().get());
        mq.addParameter("subject", msg.subject().get());
        mq.addParameter("posturl", root.linkFormatter().makePostUrl(topic.getId(), topic.subject().get(), msg.getId()));
        mq.addParameter("message", renderedText);
        mq.addParameter("author", User(root, author).getScreenName());
        mq.send(individualReceivers);
    }

    // -- Forum --
    // Crosspost?
    afl::data::IntegerList_t alsoPostedTo;
    if (msg.getId() == topic.firstPostingId().get()) {
        topic.alsoPostedTo().getAll(alsoPostedTo);
    }

    // Get forum watchers
    StringList_t forumWatchers;
    if (alsoPostedTo.empty()) {
        forum.watchers().getAll(forumWatchers);
    } else {
        afl::net::redis::StringSetOperation op(forum.watchers().merge(Forum(root, alsoPostedTo[0]).watchers()));
        for (size_t i = 1; i < alsoPostedTo.size(); ++i) {
            op.andAlso(Forum(root, alsoPostedTo[i]).watchers());
        }
        op.getAll(forumWatchers);
    }
    std::sort(forumWatchers.begin(), forumWatchers.end());

    // Find forum watchers that have not been notified and mark them.
    // The author of the posting will not be notified.
    individualReceivers.clear();
    groupReceivers.clear();
    for (StringList_t::size_type i = 0; i < forumWatchers.size(); ++i) {
        if (forumWatchers[i] != author
            && !std::binary_search(topicWatchers.begin(), topicWatchers.end(), forumWatchers[i])
            && root.checkUserPermission(readPermissions, forumWatchers[i]))
        {
            User u(root, forumWatchers[i]);
            if (u.isWatchIndividual()) {
                individualReceivers.push_back("user:" + forumWatchers[i]);
            } else if (u.notifiedForums().add(forum.getId())) {
                groupReceivers.push_back("user:" + forumWatchers[i]);
            } else {
                // No notification for you
            }
        }
    }

    // Send message. We do not use a uniquifier here, because a future
    // message will have different receivers.
    if (!groupReceivers.empty()) {
        mq.startMessage("talk-forum", afl::base::Nothing);
        mq.addParameter("forum", forum.name().get());
        mq.addParameter("subject", msg.subject().get());
        mq.addParameter("posturl", root.linkFormatter().makePostUrl(topic.getId(), topic.subject().get(), msg.getId()));
        mq.send(groupReceivers);
    }
    if (!individualReceivers.empty()) {
        server::talk::render::Context ctx(author);
        ctx.setMessageId(msg.getId());

        server::talk::render::Options opts;
        opts.setBaseUrl(root.config().baseUrl);
        opts.setFormat("mail");

        const String_t renderedText = server::talk::render::renderText(msg.text().get(), ctx, opts, root);

        mq.startMessage("talk-forum-message", afl::base::Nothing);
        mq.addParameter("forum", forum.name().get());
        mq.addParameter("subject", msg.subject().get());
        mq.addParameter("posturl", root.linkFormatter().makePostUrl(topic.getId(), topic.subject().get(), msg.getId()));
        mq.addParameter("message", renderedText);
        mq.addParameter("author", User(root, author).getScreenName());
        mq.send(individualReceivers);
    }
}


// Notify a private message.
void
server::talk::notifyPM(UserPM& msg, const afl::data::StringList_t& notifyIndividual, const afl::data::StringList_t& notifyGroup, Root& root, server::interface::MailQueue& mq)
{
    // Send notification with message
    if (!notifyIndividual.empty()) {
        const String_t author = msg.author().get();
        server::talk::render::Context ctx(author);
        server::talk::render::Options opts;
        opts.setBaseUrl(root.config().baseUrl);
        opts.setFormat("mail");

        mq.startMessage("talk-pm-message", afl::base::Nothing);
        mq.addParameter("subject", msg.subject().get());
        mq.addParameter("author", User(root, author).getScreenName());
        mq.addParameter("id", afl::string::Format("%d", msg.getId()));
        mq.addParameter("message", server::talk::render::renderText(msg.text().get(), ctx, opts, root));
        mq.send(notifyIndividual);
    }

    // Send notification without message
    if (!notifyGroup.empty()) {
        mq.startMessage("talk-pm", afl::base::Nothing);
        mq.send(notifyGroup);
    }
}
