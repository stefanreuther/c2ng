/**
  *  \file test/server/host/installertest.cpp
  *  \brief Test for server::host::Installer
  */

#include "server/host/installer.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/redis/integersetkey.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.host.Installer:isPreciousFile", a)
{
    TestHarness h;
    server::host::Installer testee(h.root());

    // precious files
    a.check("01", testee.isPreciousFile("fizz.bin"));
    a.check("02", testee.isPreciousFile("vpa1.db"));
    a.check("03", testee.isPreciousFile("team11.cc"));
    a.check("04", testee.isPreciousFile("vpa999.db"));

    // "near matches"
    a.check("11", !testee.isPreciousFile("vpa.db"));
    a.check("12", !testee.isPreciousFile("vpabc.db"));
    a.check("13", !testee.isPreciousFile("vpa1b.db"));
    a.check("14", !testee.isPreciousFile("vpa1.dat"));
    a.check("15", !testee.isPreciousFile("config3.cc"));
    a.check("16", !testee.isPreciousFile("config.ini"));
    a.check("17", !testee.isPreciousFile("STAT.CC"));

    // managed files
    a.check("21", !testee.isPreciousFile("player3.rst"));
    a.check("22", !testee.isPreciousFile("hullspec.dat"));
    a.check("23", !testee.isPreciousFile("hullfunc.dat"));
    a.check("24", !testee.isPreciousFile("pconfig.src"));
}
