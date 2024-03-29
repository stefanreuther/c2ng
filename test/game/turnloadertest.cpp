/**
  *  \file test/game/turnloadertest.cpp
  *  \brief Test for game::TurnLoader
  */

#include "game/turnloader.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.TurnLoader:interface")
{
    class Tester : public game::TurnLoader {
     public:
        virtual PlayerStatusSet_t getPlayerStatus(int /*player*/, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            { return PlayerStatusSet_t(); }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, std::auto_ptr<game::Task_t> then)
            { return then; }
        virtual String_t getProperty(Property /*p*/)
            { return String_t(); }
    };
    Tester t;
}

/** Test getDefaultPlayer(). */
AFL_TEST("game.TurnLoader:getDefaultPlayer", a)
{
    // Tester class that reports a given PlayerStatusSet_t list
    class Tester : public game::TurnLoader {
     public:
        Tester(afl::base::Memory<const PlayerStatusSet_t> data)
            : m_data(data)
            { }
        virtual PlayerStatusSet_t getPlayerStatus(int player, String_t& /*extra*/, afl::string::Translator& /*tx*/) const
            {
                if (const PlayerStatusSet_t* p = m_data.at(player-1)) {
                    return *p;
                } else {
                    return PlayerStatusSet_t();
                }
            }
        virtual std::auto_ptr<game::Task_t> loadCurrentTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveCurrentTurn(const game::Turn& /*turn*/, const game::Game& /*game*/, game::PlayerSet_t /*players*/, SaveOptions_t /*opts*/, const game::Root& /*root*/, game::Session& /*session*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual void getHistoryStatus(int /*player*/, int /*turn*/, afl::base::Memory<HistoryStatus> /*status*/, const game::Root& /*root*/)
            { }
        virtual std::auto_ptr<game::Task_t> loadHistoryTurn(game::Turn& /*turn*/, game::Game& /*game*/, int /*player*/, int /*turnNumber*/, game::Root& /*root*/, std::auto_ptr<game::StatusTask_t> then)
            { return game::makeConfirmationTask(false, then); }
        virtual std::auto_ptr<game::Task_t> saveConfiguration(const game::Root& /*root*/, std::auto_ptr<game::Task_t> then)
            { return then; }
        virtual String_t getProperty(Property /*p*/)
            { return String_t(); }
     private:
        afl::base::Memory<const PlayerStatusSet_t> m_data;
    };

    // Abbreviations
    const game::PlayerSet_t ALL_PLAYERS = game::PlayerSet_t::allUpTo(10);
    typedef game::TurnLoader::PlayerStatusSet_t PlayerStatusSet_t;

    // No player
    {
        const PlayerStatusSet_t set[] = { PlayerStatusSet_t() };
        Tester t(set);
        a.checkEqual("01. no player", t.getDefaultPlayer(ALL_PLAYERS), 0);
    }

    // Single available player
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("11. single player", t.getDefaultPlayer(ALL_PLAYERS), 3);
    }

    // Multiple available players (ambiguous)
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("21. multiple players", t.getDefaultPlayer(ALL_PLAYERS), 0);
    }

    // Multiple available players, but one is primary
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available) + game::TurnLoader::Primary,
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("31. primary player", t.getDefaultPlayer(ALL_PLAYERS), 4);
    }

    // Multiple available players, different order
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Available) + game::TurnLoader::Primary,
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("41. primary player", t.getDefaultPlayer(ALL_PLAYERS), 2);
    }

    // Multiple primaries, ambiguous
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Available) + game::TurnLoader::Primary,
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t(game::TurnLoader::Available) + game::TurnLoader::Primary,
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("51. multiple primaries", t.getDefaultPlayer(ALL_PLAYERS), 0);
    }

    // Primary but not available is ignored
    {
        const PlayerStatusSet_t set[] = {
            PlayerStatusSet_t(),
            PlayerStatusSet_t(game::TurnLoader::Primary),
            PlayerStatusSet_t(game::TurnLoader::Available),
            PlayerStatusSet_t()
        };
        Tester t(set);
        a.checkEqual("61. unavailable primary", t.getDefaultPlayer(ALL_PLAYERS), 3);
    }
}
