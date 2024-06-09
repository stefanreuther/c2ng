/**
  *  \file test/game/roottest.cpp
  *  \brief Test for game::Root
  */

#include "game/root.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/registrationkey.hpp"
#include "game/specificationloader.hpp"
#include "game/stringverifier.hpp"
#include "game/test/counter.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turnloader.hpp"

/** Simple test. */
AFL_TEST("game.Root", a)
{
    // Prepare directory
    afl::base::Ref<afl::io::Directory> gameDirectory(afl::io::InternalDirectory::create("game"));

    // SpecificationLoader
    afl::base::Ref<game::SpecificationLoader> specLoader(*new game::test::SpecificationLoader());

    // Host version
    game::HostVersion hostVersion(game::HostVersion::PHost, MKVERSION(4,0,0));

    // Registration key
    std::auto_ptr<game::RegistrationKey> regKey(new game::test::RegistrationKey(game::RegistrationKey::Unknown, 100));

    // StringVerifier
    std::auto_ptr<game::StringVerifier> stringVerifier(new game::test::StringVerifier());

    // Charset
    std::auto_ptr<afl::charset::Charset> charset(new afl::charset::Utf8Charset());

    // Build a root
    game::Root testee(gameDirectory, specLoader, hostVersion, regKey, stringVerifier, charset, game::Root::Actions_t());
    const game::Root& croot(testee);

    // Verify it
    a.checkEqual  ("01. gameDirectory",       &testee.gameDirectory(), &*gameDirectory);
    a.checkEqual  ("02. specificationloader", &testee.specificationLoader(), &*specLoader);
    a.checkEqual  ("03. hostVersion",         testee.hostVersion().getKind(), game::HostVersion::PHost);
    a.checkEqual  ("04. hostVersion",         testee.hostVersion().getVersion(), MKVERSION(4,0,0));
    a.checkNonNull("05. registrationKey",     dynamic_cast<game::test::RegistrationKey*>(&testee.registrationKey()));
    a.checkNonNull("06. stringVerifier",      dynamic_cast<const game::test::StringVerifier*>(&testee.stringVerifier()));
    a.checkNonNull("07. charset",             dynamic_cast<afl::charset::Utf8Charset*>(&testee.charset()));
    a.checkNull   ("08. turnLoader",          testee.getTurnLoader().get());
    a.check       ("09. getPossibleActions",  testee.getPossibleActions().empty());

    // Verify accessors
    a.checkEqual  ("11. hostVersion",       &testee.hostVersion(), &croot.hostVersion());
    a.checkEqual  ("12. hostConfiguration", &testee.hostConfiguration(), &croot.hostConfiguration());
    a.checkEqual  ("13. flakConfiguration", &testee.flakConfiguration(), &croot.flakConfiguration());
    a.checkEqual  ("14. userConfiguration", &testee.userConfiguration(), &croot.userConfiguration());
    a.checkNonNull("15. charset",           dynamic_cast<afl::charset::Utf8Charset*>(&croot.charset()));
    a.checkEqual  ("16. playerList",        &testee.playerList(), &croot.playerList());

    // Set a TurnLoader
    class NullTurnLoader : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, afl::sys::LogListener& /*log*/, afl::string::Translator& /*tx*/, std::auto_ptr<game::Task_t> then)
            { return then; }
        virtual String_t getProperty(Property /*p*/)
            { return String_t(); }
    };
    afl::base::Ref<game::TurnLoader> turnLoader(*new NullTurnLoader());
    testee.setTurnLoader(turnLoader.asPtr());
    a.checkEqual("21. turnLoader", testee.getTurnLoader().get(), &*turnLoader);

    // Finally, verify notifications
    testee.notifyListeners();

    using game::test::Counter;
    Counter c;
    testee.hostConfiguration().sig_change.add(&c, &Counter::increment);
    testee.hostConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.userConfiguration().sig_change.add(&c, &Counter::increment);
    testee.userConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.notifyListeners();
    a.checkEqual("31. notification count", c.get(), 2);
}
