/**
  *  \file test/game/proxy/pluginmanagerproxytest.cpp
  *  \brief Test for game::proxy::PluginManagerProxy
  */

#include "game/proxy/pluginmanagerproxy.hpp"

#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include <cstring>

namespace {
    class Receiver {
     public:
        Receiver()
            : m_list(), m_details("?", "?", util::plugin::Manager::NotLoaded), m_numLists(), m_numDetails()
            { }

        void onList(const game::proxy::PluginManagerProxy::Infos_t& list)
            {
                m_list = list;
                ++m_numLists;
            }

        void onDetails(const game::proxy::PluginManagerProxy::Details_t& d)
            {
                m_details = d;
                ++m_numDetails;
            }

        void connect(game::proxy::PluginManagerProxy& proxy)
            {
                proxy.sig_list.add(this, &Receiver::onList);
                proxy.sig_details.add(this, &Receiver::onDetails);
            }

        game::proxy::PluginManagerProxy::Infos_t m_list;
        game::proxy::PluginManagerProxy::Details_t m_details;
        int m_numLists;
        int m_numDetails;
    };
}

/** Test general information requests. */
AFL_TEST("game.proxy.PluginManagerProxy:request", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("A"));
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("B"));

    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    Receiver recv;
    recv.connect(testee);

    // requestList()
    testee.requestList();
    t.sync();
    ind.processQueue();
    a.checkEqual("01. m_numLists", recv.m_numLists, 1);
    a.checkEqual("02. size", recv.m_list.size(), 2U);
    a.checkEqual("03. id 0", recv.m_list[0].id, "A");
    a.checkEqual("04. id 1", recv.m_list[1].id, "B");

    // requestDetails()
    testee.requestDetails("B");
    t.sync();
    ind.processQueue();
    a.checkEqual("11. m_numDetails", recv.m_numDetails, 1);
    a.checkEqual("12. id", recv.m_details.id, "B");
}

/** Test request debouncing.
    If we send multiple requests, ideally only a single response shall arrive (no response queueing). */
AFL_TEST("game.proxy.PluginManagerProxy:debounce", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("A"));
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("B"));

    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    Receiver recv;
    recv.connect(testee);

    // Request varying data
    for (int i = 1; i < 100; ++i) {
        testee.requestDetails("A");
        testee.requestDetails("B");
    }

    // PluginManagerProxy will perform an unspecified number of requests before producing a callback.
    // However, we want it to be significantly fewer than the number of requests.
    for (int i = 1; i < 10; ++i) {
        t.sync();
        ind.processQueue();
    }

    // Number of responses shall be even lower (ideally, 1).
    a.check("01. m_numDetails", recv.m_numDetails <= 3);
    a.checkEqual("02. id", recv.m_details.id, "B");
}

/** Test installation, happy case. */
AFL_TEST("game.proxy.PluginManagerProxy:install", a)
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "description = Lorem ipsum dolor sit amet, consectetuer adipiscing elit.\n"
        "description = Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula.\n"
        "helpfile = h.xml\n";
    const char*const HELP_CONTENT = "<help />";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));
    fs.openFile("/in/h.xml", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(HELP_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, true);
    a.checkEqual("02. isUpdate", info.isUpdate, false);
    a.checkEqual("03. ambiguity", info.ambiguity, util::plugin::Installer::NoPlugin);
    // errorMessage not relevant
    a.checkEqual("04. fileName", info.fileName, "/in/t.c2p");
    a.checkEqual("05. fileTitle", info.fileTitle, "t.c2p");
    // altName, altTitle not relevant
    a.checkEqual("06. pluginId", info.pluginId, "T");
    a.checkEqual("07. pluginName", info.pluginName, "TestPlugin");
    a.checkEqual("08. pluginDescription", info.pluginDescription, "Lorem ipsum dolor sit amet, consectetuer adipiscing elit.");  // only first line reported here
    a.check("09. conflicts", !info.conflicts.isValid());

    AFL_CHECK_THROWS(a("11. openFile"), fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // Perform the installation; verify result
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    a.checkEqual("21. isValid", result.isValid, true);
    a.checkEqual("22. pluginId", result.pluginId, "T");

    // Verify that plugin was installed
    // - content needs to be present
    uint8_t found[100];
    afl::base::ConstBytes_t foundBytes(found);
    AFL_CHECK_SUCCEEDS(a("31. file content"), foundBytes.trim(fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead)->read(found)));
    a.checkEqual("32. size", foundBytes.size(), std::strlen(HELP_CONTENT));
    a.checkEqualContent("33. content", foundBytes, afl::string::toBytes(HELP_CONTENT));

    // - plugin file needs to be present (don't check content, it can be rewritten)
    AFL_CHECK_SUCCEEDS(a("41. openFile"), fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead));
}

/** Test installation, error: no directory set.
    Installation fails immediately.
    This is not a relevant error condition. */
AFL_TEST("game.proxy.PluginManagerProxy:install:error:no-directory", a)
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT = "name = TestPlugin\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    // no setPluginDirectoryName()

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, false);
    // We don't produce any specific error message here
}

/** Test installation, error: file does not exist.
    Installation fails.
    The error message should be provided to the user. */
AFL_TEST("game.proxy.PluginManagerProxy:install:error:no-file", a)
{
    // Session with no file system
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, false);
    a.checkDifferent("02. errorMessage", info.errorMessage, ""); // should be ENOENT message from file system
}

/** Test installation, error: file not understood.
    Installation fails.
    This is the normal trigger for an failure without error message. */
AFL_TEST("game.proxy.PluginManagerProxy:install:error:bad-file", a)
{
    // A file system containing a file
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.xyz", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("whatever"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.xyz");
    a.checkEqual("01. isValid", info.isValid, false);
    // We don't produce any specific error message here
}

/** Test installation: "provides" conflict (provided feature already exists).
    Conflict is reported. */
AFL_TEST("game.proxy.PluginManagerProxy:install:provided", a)
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "provides = QQQ\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");
    t.session().plugins().addNewPlugin(new util::plugin::Plugin("QQQ"));

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, true);
    a.checkEqual("02. isUpdate", info.isUpdate, false);

    // Must report a nonempty error message that mentions QQQ
    a.check("11. conflicts", info.conflicts.isValid());
    a.checkDifferent("12. conflict", *info.conflicts.get(), "");
    a.checkDifferent("13. conflict", info.conflicts.get()->find("QQQ"), String_t::npos);
}

/** Test installation: "requires" conflict (required feature does not exist).
    Conflict is reported. */
AFL_TEST("game.proxy.PluginManagerProxy:install:conflict:required", a)
{
    // A file system containing the plugin
    const char*const PLUGIN_CONTENT =
        "name = TestPlugin\n"
        "requires = ZZZ\n";

    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes(PLUGIN_CONTENT));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, true);
    a.checkEqual("02. isUpdate", info.isUpdate, false);

    // Must report a nonempty error message that mentions QQQ
    a.check("11. conflicts", info.conflicts.isValid());
    a.checkDifferent("12. conflict", *info.conflicts.get(), "");
    a.checkDifferent("13. conflict", info.conflicts.get()->find("ZZZ"), String_t::npos);
}

/** Test installation: single alternative.
    Alternative is reported. */
AFL_TEST("game.proxy.PluginManagerProxy:install:one-alternative", a)
{
    // File system content
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/h.res", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/h.res");
    a.checkEqual("01. isValid", info.isValid, true);
    a.checkEqual("02. isUpdate", info.isUpdate, false);
    a.checkEqual("03. ambiguity", info.ambiguity, util::plugin::Installer::OnePlugin);
    // errorMessage not relevant
    a.checkEqual("04. fileName", info.fileName, "/in/h.res");
    a.checkEqual("05. fileTitle", info.fileTitle, "h.res");
    a.checkEqual("06. altName", info.altName, "/in/t.c2p");
    a.checkEqual("07. altTitle", info.altTitle, "t.c2p");
    a.checkEqual("08. pluginId", info.pluginId, "H");
    a.check("09. conflicts", !info.conflicts.isValid());
}

/** Test installation: multiple alternatives.
    Availability of alternatives is reported. */
AFL_TEST("game.proxy.PluginManagerProxy:install:multiple-alternatives", a)
{
    // File system content
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/s.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));
    fs.openFile("/in/h.res", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("..."));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/h.res");
    a.checkEqual("01. isValid", info.isValid, true);
    a.checkEqual("02. isUpdate", info.isUpdate, false);
    a.checkEqual("03. ambiguity", info.ambiguity, util::plugin::Installer::MultiplePlugins);
    // errorMessage not relevant
    a.checkEqual("04. fileName", info.fileName, "/in/h.res");
    a.checkEqual("05. fileTitle", info.fileTitle, "h.res");
    // altName, altTitle not relevant
    a.checkEqual("06. pluginId", info.pluginId, "H");
    a.check("07. conflicts", !info.conflicts.isValid());
}

/** Test installation: missing payload file.
    This fails the installation with an exception. */
AFL_TEST("game.proxy.PluginManagerProxy:install:error:missing-file", a)
{
    // A file system containing the plugin
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("helpfile = h.xml\n"));
    // no h.xml

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, true);

    // Perform the installation; verify result. Error message must reference the missing file name.
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    a.checkEqual("11. isValid", result.isValid, false);
    a.checkDifferent("12. errorMessage", result.errorMessage.find("h.xml"), String_t::npos);

    // Verify that plugin was not installed
    AFL_CHECK_THROWS(a("21. openFile"), fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test installation: bad sequence (cancellation). */
AFL_TEST("game.proxy.PluginManagerProxy:install:error:sequence", a)
{
    // A file system containing the plugin
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/in");
    fs.openFile("/in/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().setPluginDirectoryName("/p");

    // Prepare the installation; verify result
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::InstallInfo info = testee.prepareInstall(ind, "/in/t.c2p");
    a.checkEqual("01. isValid", info.isValid, true);

    // Sequence violation
    testee.cancelInstallation();

    // Try to perform the installation; must fail.
    game::proxy::PluginManagerProxy::InstallResult result = testee.doInstall(ind);
    a.checkEqual("11. isValid", result.isValid, false);

    // Verify that plugin was not installed
    AFL_CHECK_THROWS(a("21. openFile"), fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
}

/** Test remove, happy case. */
AFL_TEST("game.proxy.PluginManagerProxy:remove", a)
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/t");
    fs.createDirectory("/p/q");
    fs.openFile("/p/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\nhelpfile = h.xml\n"));
    fs.openFile("/p/t/h.xml", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("<help />"));
    fs.openFile("/p/q.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = other\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    a.checkEqual("01. isValid", preResult.isValid, true);
    AFL_CHECK_SUCCEEDS(a("02. openFile"), fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead));  // file still exists

    // Remove
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    a.checkEqual("11. isValid", result.isValid, true);

    // Files gone
    AFL_CHECK_THROWS(a("21. openFile"), fs.openFile("/p/t.c2p", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);
    AFL_CHECK_THROWS(a("22. openFile"), fs.openFile("/p/t/h.xml", afl::io::FileSystem::OpenRead), afl::except::FileProblemException);

    // Only plugin Q remains
    std::vector<util::plugin::Plugin*> plugins;
    t.session().plugins().enumPlugins(plugins, true);
    a.checkEqual("31. size", plugins.size(), 1U);
    a.checkEqual("32. getId", plugins[0]->getId(), "Q");
}

/** Test remove, plugin is required by someone else. */
AFL_TEST("game.proxy.PluginManagerProxy:remove:error:dependee", a)
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/ttt");
    fs.createDirectory("/p/q");
    fs.openFile("/p/ttt.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plug\nrequires = q\n"));
    fs.openFile("/p/q.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = other\n"));

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "Q");
    a.checkEqual("01. isValid", preResult.isValid, false);
    a.checkDifferent("02. errorMessage", preResult.errorMessage.find("TTT"), String_t::npos);
}

/** Test remove, not all files exist. */
AFL_TEST("game.proxy.PluginManagerProxy:remove:error:missing-files", a)
{
    // A file system containing installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");
    fs.createDirectory("/p/t");
    fs.openFile("/p/t.c2p", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("name = plugin\nhelpfile = h.xml\n"));
    // no h.xml file

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    a.checkEqual("01. isValid", preResult.isValid, true);

    // Remove
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    a.checkEqual("11. isValid", result.isValid, false);
}

/** Test remove, desired name does not exist. */
AFL_TEST("game.proxy.PluginManagerProxy:remove:error:bad-name", a)
{
    // A file system with no installed plugins
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/p");

    // Setup
    game::test::SessionThread t(fs);
    game::test::WaitIndicator ind;
    t.session().plugins().findPlugins(*fs.openDirectory("/p"));
    t.session().setPluginDirectoryName("/p");

    // Prepare reports failure
    game::proxy::PluginManagerProxy testee(t.gameSender(), ind);
    game::proxy::PluginManagerProxy::RemoveResult preResult = testee.prepareRemove(ind, "T");
    a.checkEqual("01. isValid", preResult.isValid, false);

    // Remove reports failure
    game::proxy::PluginManagerProxy::RemoveResult result = testee.doRemove(ind, "T");
    a.checkEqual("11. isValid", result.isValid, false);
}
