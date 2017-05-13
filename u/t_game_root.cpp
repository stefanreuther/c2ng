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
#include "helper/counter.hpp"

/** Simple test. */
void
TestGameRoot::testIt()
{
    // Prepare directories
    afl::base::Ref<afl::io::Directory> specificationDirectory(afl::io::InternalDirectory::create("spec"));
    afl::base::Ref<afl::io::Directory> gameDirectory(afl::io::InternalDirectory::create("game"));

    // SpecificationLoader
    class NullSpecificationLoader : public game::SpecificationLoader {
     public:
        virtual void loadShipList(game::spec::ShipList& /*list*/, game::Root& /*root*/)
            { }
    };
    afl::base::Ref<game::SpecificationLoader> specLoader(*new NullSpecificationLoader());

    // Host version
    game::HostVersion hostVersion(game::HostVersion::PHost, MKVERSION(4,0,0));

    // Registration key
    class NullRegistrationKey : public game::RegistrationKey {
     public:
        virtual Status getStatus() const
            { return Status(); }
        virtual String_t getLine(Line /*which*/) const
            { return String_t(); }
        virtual bool setLine(Line /*which*/, String_t /*value*/)
            { return false; }
    };
    std::auto_ptr<game::RegistrationKey> regKey(new NullRegistrationKey());

    // StringVerifier
    class NullStringVerifier : public game::StringVerifier {
     public:
        virtual bool isValidString(Context /*ctx*/, const String_t& /*text*/)
            { return false; }
        virtual bool isValidCharacter(Context /*ctx*/, afl::charset::Unichar_t /*ch*/)
            { return false; }
        virtual size_t getMaxStringLength(Context /*ctx*/)
            { return 0; }
        virtual NullStringVerifier* clone() const
            { return new NullStringVerifier(); }
    };
    std::auto_ptr<game::StringVerifier> stringVerifier(new NullStringVerifier());
    
    // Build a root
    game::Root testee(specificationDirectory, gameDirectory, specLoader, hostVersion, regKey, stringVerifier);
    const game::Root& croot(testee);

    // Verify it
    TS_ASSERT_EQUALS(&testee.specificationDirectory(), &*specificationDirectory);
    TS_ASSERT_EQUALS(&testee.gameDirectory(), &*gameDirectory);
    TS_ASSERT_EQUALS(&testee.specificationLoader(), &*specLoader);
    TS_ASSERT_EQUALS(testee.hostVersion().getKind(), game::HostVersion::PHost);
    TS_ASSERT_EQUALS(testee.hostVersion().getVersion(), MKVERSION(4,0,0));
    TS_ASSERT(dynamic_cast<NullRegistrationKey*>(&testee.registrationKey()) != 0);
    TS_ASSERT(dynamic_cast<NullStringVerifier*>(&testee.stringVerifier()) != 0);
    TS_ASSERT(testee.getTurnLoader().get() == 0);

    // Verify accessors
    TS_ASSERT_EQUALS(&testee.hostVersion(), &croot.hostVersion());
    TS_ASSERT_EQUALS(&testee.hostConfiguration(), &croot.hostConfiguration());
    TS_ASSERT_EQUALS(&testee.userConfiguration(), &croot.userConfiguration());
    TS_ASSERT_EQUALS(&testee.playerList(), &croot.playerList());

    // Set a TurnLoader
    class NullTurnLoader : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual void loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/)
            { }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { }
        virtual void loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/)
            { }
    };
    afl::base::Ref<game::TurnLoader> turnLoader(*new NullTurnLoader());
    testee.setTurnLoader(turnLoader.asPtr());
    TS_ASSERT_EQUALS(testee.getTurnLoader().get(), &*turnLoader);

    // Finally, verify notifications
    testee.notifyListeners();

    Counter c;
    testee.hostConfiguration().sig_change.add(&c, &Counter::increment);
    testee.hostConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.userConfiguration().sig_change.add(&c, &Counter::increment);
    testee.userConfiguration().setOption("foo", "bar", game::config::ConfigurationOption::User);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(c.get(), 2);
}

