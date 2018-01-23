/**
  *  \file server/host/root.hpp
  *  \brief Class server::host::Root
  */
#ifndef C2NG_SERVER_HOST_ROOT_HPP
#define C2NG_SERVER_HOST_ROOT_HPP

#include "afl/sys/log.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/redis/subtree.hpp"
#include "server/host/configuration.hpp"
#include "server/types.hpp"
#include "afl/sys/mutex.hpp"
#include "server/interface/talkforum.hpp"
#include "server/interface/mailqueue.hpp"
#include "server/host/gamearbiter.hpp"
#include "util/processrunner.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/time.hpp"
#include "util/randomnumbergenerator.hpp"
#include "afl/net/redis/stringlistkey.hpp"
#include "server/interface/sessionrouter.hpp"
#include "server/common/root.hpp"

namespace server { namespace host {

    class Cron;
    class TalkListener;

    class Root : public server::common::Root {
     public:
        /** Tool tree.
            The tool tree contains information for a category of tools. */
        class ToolTree {
         public:
            ToolTree(afl::net::redis::Subtree tree);
            afl::net::redis::StringSetKey all();
            afl::net::redis::HashKey byName(String_t name);
            afl::net::redis::StringKey defaultName();
         private:
            afl::net::redis::Subtree m_tree;
        };


        /** Constructor.
            All provided references must out-live the Root object.
            \param db         CommandHandler accessing a database (redis)
            \param hostFile   CommandHandler accessing the host file server
            \param userFile   CommandHandler accessing the user file server
            \param mailQueue  Interface to mail queue
            \param checkturnRunner ProcessRunner to use for running turn checker
            \param fs         File system
            \param config     Configuration */
        Root(afl::net::CommandHandler& db,
             afl::net::CommandHandler& hostFile,
             afl::net::CommandHandler& userFile,
             server::interface::MailQueue& mailQueue,
             util::ProcessRunner& checkturnRunner,
             afl::io::FileSystem& fs,
             const Configuration& config);

        /** Destructor. */
        ~Root();

        /** Access logger.
            Attach a listener to receive log messages.
            \return logger */
        afl::sys::Log& log();

        /** Mutex.
            Acquire before using any of the microservice connections. */
        afl::sys::Mutex& mutex();

        /** Configure reconnect behaviour.
            Call this before executing a user command.

            A user command will translate into a sequence of commands to other services.
            Those commands might carry state (most notably, a user context).
            Blindly reconnecting on every disconnection would lose the state.
            We therefore only reconnect once for each sequence.
            Mid-way connection loss is a failure that is propagated to the caller. */
        void configureReconnect();

        /** Set cron (scheduler) implementation.
            The host server can run with or without a scheduler, mostly for testing. */
        void setCron(Cron* p);

        /** Set talk (forum) implementation. */
        void setForum(TalkListener* p);

        /** Set router implementation.
            The host server can run with or without a session router. */
        void setRouter(server::interface::SessionRouter* p);

        afl::net::CommandHandler& hostFile();
        afl::net::CommandHandler& userFile();

        TalkListener* getForum();
        server::interface::MailQueue& mailQueue();

        GameArbiter& arbiter();

        const Configuration& config() const;
        util::RandomNumberGenerator& rng();

        util::ProcessRunner& checkturnRunner();
        afl::io::FileSystem& fileSystem();

        Time_t getTime();
        afl::sys::Time getSystemTimeFromTime(Time_t t);

        Cron* getCron();
        void handleGameChange(int32_t gameId);

        server::interface::SessionRouter* getRouter();
        void tryCloseRouterSessions(String_t key);


        /*
         *  Database Schema
         */

        /** Access root of "host" tools.
            \return handle */
        ToolTree hostRoot();

        /** Access root of "master" tools.
            \return handle */
        ToolTree masterRoot();

        /** Access root of "ship list" tools.
            \return handle */
        ToolTree shipListRoot();

        /** Access root of "tools".
            \return handle */
        ToolTree toolRoot();

        /** Access set of active users.
            \return set */
        afl::net::redis::StringSetKey activeUsers();

        /** Access global history.
            \return list. Most-current message is at front. */
        afl::net::redis::StringListKey globalHistory();


     private:
        afl::sys::Log m_log;
        afl::sys::Mutex m_mutex;

        afl::net::CommandHandler& m_db;
        afl::net::CommandHandler& m_hostFile;
        afl::net::CommandHandler& m_userFile;

        server::interface::MailQueue& m_mailQueue;

        GameArbiter m_arbiter;

        util::ProcessRunner& m_checkturnRunner;
        afl::io::FileSystem& m_fileSystem;

        TalkListener* m_pTalkListener;
        Cron* m_pCron;
        server::interface::SessionRouter* m_pRouter;

        Configuration m_config;
        util::RandomNumberGenerator m_rng;
    };

} }

#endif
