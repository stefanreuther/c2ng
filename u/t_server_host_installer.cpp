/**
  *  \file u/t_server_host_installer.cpp
  *  \brief Test for server::host::Installer
  */

#include "server/host/installer.hpp"

#include "t_server_host.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/host/configuration.hpp"
#include "server/host/root.hpp"
#include "server/interface/mailqueueclient.hpp"
#include "util/processrunner.hpp"

namespace {
    class TestHarness {
     public:
        TestHarness()
            : m_db(), m_hostFile(), m_userFile(), m_null(), m_mail(m_null), m_runner(), m_fs(),
              m_root(m_db, m_hostFile, m_userFile, m_mail, m_runner, m_fs, server::host::Configuration())
            { }

        server::host::Root& root()
            { return m_root; }

        afl::net::CommandHandler& db()
            { return m_db; }

     private:
        afl::net::redis::InternalDatabase m_db;
        server::file::InternalFileServer m_hostFile;
        server::file::InternalFileServer m_userFile;
        afl::net::NullCommandHandler m_null;
        server::interface::MailQueueClient m_mail;
        util::ProcessRunner m_runner;
        afl::io::NullFileSystem m_fs;
        server::host::Root m_root;
    };
}

/** Test isPreciousFile(). */
void
TestServerHostInstaller::testPrecious()
{
    TestHarness h;
    server::host::Installer testee(h.root());

    // precious files
    TS_ASSERT(testee.isPreciousFile("fizz.bin"));
    TS_ASSERT(testee.isPreciousFile("vpa1.db"));
    TS_ASSERT(testee.isPreciousFile("team11.cc"));
    TS_ASSERT(testee.isPreciousFile("vpa999.db"));

    // "near matches"
    TS_ASSERT(!testee.isPreciousFile("vpa.db"));
    TS_ASSERT(!testee.isPreciousFile("vpabc.db"));
    TS_ASSERT(!testee.isPreciousFile("vpa1b.db"));
    TS_ASSERT(!testee.isPreciousFile("vpa1.dat"));
    TS_ASSERT(!testee.isPreciousFile("config3.cc"));
    TS_ASSERT(!testee.isPreciousFile("config.ini"));
    TS_ASSERT(!testee.isPreciousFile("STAT.CC"));

    // managed files
    TS_ASSERT(!testee.isPreciousFile("player3.rst"));
    TS_ASSERT(!testee.isPreciousFile("hullspec.dat"));
    TS_ASSERT(!testee.isPreciousFile("hullfunc.dat"));
    TS_ASSERT(!testee.isPreciousFile("pconfig.src"));
}

