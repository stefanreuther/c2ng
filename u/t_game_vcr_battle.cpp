/**
  *  \file u/t_game_vcr_battle.cpp
  *  \brief Test for game::vcr::Battle
  */

#include <vector>
#include "game/vcr/battle.hpp"

#include "t_game_vcr.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/vcr/object.hpp"

/** Interface test. */
void
TestGameVcrBattle::testIt()
{
    class Tester : public game::vcr::Battle {
     public:
        virtual size_t getNumObjects() const
            { return 0; }
        virtual const game::vcr::Object* getObject(size_t /*slot*/, bool /*after*/) const
            { return 0; }
        virtual int getOutcome(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, size_t /*slot*/)
            { return 0; }
        virtual Playability getPlayability(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/)
            { return IsDamaged; }
        virtual void prepareResult(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, int /*resultLevel*/)
            { }
        virtual String_t getAlgorithmName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual bool isESBActive(const game::config::HostConfiguration& /*config*/) const
            { return false; }
        virtual bool getPosition(game::map::Point&) const
            { return false; }
    };
    Tester t;
}

/** Test getDescription(). */
void
TestGameVcrBattle::testDescription()
{
    /* Simple battle that stores just a bunch of objects */
    class BattleMock : public game::vcr::Battle {
     public:
        virtual size_t getNumObjects() const
            { return m_objects.size(); }
        virtual const game::vcr::Object* getObject(size_t slot, bool /*after*/) const
            {
                if (slot < m_objects.size()) {
                    return &m_objects[slot];
                } else {
                    return 0;
                }
            }
        virtual int getOutcome(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, size_t /*slot*/)
            { return 0; }
        virtual Playability getPlayability(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/)
            { return Playability(); }
        virtual void prepareResult(const game::config::HostConfiguration& /*config*/, const game::spec::ShipList& /*shipList*/, int /*resultLevel*/)
            { }
        virtual String_t getAlgorithmName(afl::string::Translator& /*tx*/) const
            { return String_t(); }
        virtual bool isESBActive(const game::config::HostConfiguration& /*config*/) const
            { return false; }
        virtual bool getPosition(game::map::Point& /*result*/) const
            { return false; }

        void addObject(int owner, String_t name)
            {
                game::vcr::Object obj;
                obj.setOwner(owner);
                obj.setName(name);
                m_objects.push_back(obj);
            }
     private:
        std::vector<game::vcr::Object> m_objects;
    };

    // Environment
    const game::PlayerList players;
    afl::string::NullTranslator tx;

    // Empty battle [error case]
    {
        BattleMock m;
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "Unknown");
    }

    // Singular battle [error case]
    {
        BattleMock m;
        m.addObject(1, "One");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "Unknown");
    }

    // Singular battle [error case]
    {
        BattleMock m;
        m.addObject(1, "One");
        m.addObject(1, "One too");
        m.addObject(1, "Also one");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "Unknown");
    }

    // Regular 1:1 battle [regular case]
    {
        BattleMock m;
        m.addObject(1, "One");
        m.addObject(2, "Two");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "One vs. Two");
    }

    // 1:n battle
    {
        BattleMock m;
        m.addObject(1, "One");
        m.addObject(2, "Two");
        m.addObject(2, "Two too");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "One vs. Player 2");
    }

    // n:m battle
    {
        BattleMock m;
        m.addObject(1, "One");
        m.addObject(1, "One too");
        m.addObject(2, "Two");
        m.addObject(2, "Two too");
        m.addObject(1, "One again");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "Player 1 vs. Player 2");
    }

    // Multiple races
    {
        BattleMock m;
        m.addObject(1, "One");
        m.addObject(2, "Two");
        m.addObject(3, "Three");
        TS_ASSERT_EQUALS(m.getDescription(players, tx), "Multiple races");
    }
}

