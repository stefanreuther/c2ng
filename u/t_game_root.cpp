/**
  *  \file u/t_game_root.cpp
  *  \brief Test for game::Root
  */

#include "game/root.hpp"

#include "t_game.hpp"
#include "afl/io/internaldirectory.hpp"
#include "game/specificationloader.hpp"
#include "game/hostversion.hpp"
#include "game/registrationkey.hpp"
#include "game/stringverifier.hpp"
#include "game/turnloader.hpp"
#include "game/test/counter.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/stringverifier.hpp"
#include "game/test/specificationloader.hpp"
#include "afl/charset/utf8charset.hpp"

/** Simple test. */
void
TestGameRoot::testIt()
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
    TS_ASSERT_EQUALS(&testee.gameDirectory(), &*gameDirectory);
    TS_ASSERT_EQUALS(&testee.specificationLoader(), &*specLoader);
    TS_ASSERT_EQUALS(testee.hostVersion().getKind(), game::HostVersion::PHost);
    TS_ASSERT_EQUALS(testee.hostVersion().getVersion(), MKVERSION(4,0,0));
    TS_ASSERT(dynamic_cast<game::test::RegistrationKey*>(&testee.registrationKey()) != 0);
    TS_ASSERT(dynamic_cast<const game::test::StringVerifier*>(&testee.stringVerifier()) != 0);
    TS_ASSERT(dynamic_cast<afl::charset::Utf8Charset*>(&testee.charset()) != 0);
    TS_ASSERT(testee.getTurnLoader().get() == 0);

    // Verify accessors
    TS_ASSERT_EQUALS(&testee.hostVersion(), &croot.hostVersion());
    TS_ASSERT_EQUALS(&testee.hostConfiguration(), &croot.hostConfiguration());
    TS_ASSERT_EQUALS(&testee.flakConfiguration(), &croot.flakConfiguration());
    TS_ASSERT_EQUALS(&testee.userConfiguration(), &croot.userConfiguration());
    TS_ASSERT(dynamic_cast<afl::charset::Utf8Charset*>(&croot.charset()) != 0);
    TS_ASSERT_EQUALS(&testee.playerList(), &croot.playerList());

    // Set a TurnLoader
    class NullTurnLoader : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, int /*player*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual String_t getProperty(Property /*p*/)
            { return String_t(); }
    };
    afl::base::Ref<game::TurnLoader> turnLoader(*new NullTurnLoader());
    testee.setTurnLoader(turnLoader.asPtr());
    TS_ASSERT_EQUALS(testee.getTurnLoader().get(), &*turnLoader);

    // Finally, verify notifications
    testee.notifyListeners();

    using game::test::Counter;
    Counter c;
    testee.hostConfiguration().sig_change.add(&c, &Counter::increment);
    testee.hostConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.userConfiguration().sig_change.add(&c, &Counter::increment);
    testee.userConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 2);
}

