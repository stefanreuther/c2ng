/**
  *  \file game/vcr/test/battle.hpp
  *  \brief Class game::vcr::test::Battle
  */
#ifndef C2NG_GAME_VCR_TEST_BATTLE_HPP
#define C2NG_GAME_VCR_TEST_BATTLE_HPP

#include "game/vcr/battle.hpp"
#include "afl/container/ptrvector.hpp"
#include "util/vector.hpp"

namespace game { namespace vcr { namespace test {

    /** VCR Battle for testing.

        Add objects using addObject().
        Modify them using getObject() if desired.

        Add groups using addGroup().
        If you do not add groups, getGroupInfo() will report default groups.

        Use the other setter methods to configure the result of other getters. */
    class Battle : public game::vcr::Battle {
     public:
        /** Constructor.
            Makes an empty battle. */
        Battle();

        /** Destructor. */
        ~Battle();

        /** Add an object.
            Adds a new object by copying the provided one as both "before" and "after" copy.
            You can modify it afterwards using getObject().
            @param obj     Object for getObject()
            @param outcome Value for getOutcome() */
        void addObject(const Object& obj, int outcome);

        /** Mutable access to object.
            @param slot  Slot [0,getNumObjects())
            @param after true to access "after", false for "before" object
            @return object; null if parameters out of range */
        Object* getObject(size_t slot, bool after);

        /** Add a group.
            Defines the result of getGroupInfo().
            If you add groups, you must add an entire set.
            If you do not add any groups, getGroupInfo() will synthesize the information.
            @param info Info */
        void addGroup(const GroupInfo& info);

        /** Set playability.
            Defines the result of getPlayability().
            Default is IsPlayable.
            @param p Playability */
        void setPlayability(Playability p);

        /** Set algorithm name.
            Defines the result of getAlgorithmName().
            Default is "Test".
            @param name Name */
        void setAlgorithmName(const String_t& name);

        /** Set status of ESB.
            Defines the result of isESBActive().
            Default is disabled.
            @param flag Flag */
        void setIsESBActive(bool flag);

        /** Set position.
            Defines the result of getPosition().
            Default is unknown.
            @param pos Position */
        void setPosition(game::map::Point pos);

        /** Set auxiliary information.
            Defines the result of getAuxiliaryInformation().
            Default is unknown.
            @param info  Which value to set
            @param value Value */
        void setAuxiliaryInformation(AuxInfo info, int32_t value);

        // Battle virtuals:
        virtual size_t getNumObjects() const;
        virtual const Object* getObject(size_t slot, bool after) const;
        virtual size_t getNumGroups() const;
        virtual GroupInfo getGroupInfo(size_t groupNr, const game::config::HostConfiguration& config) const;
        virtual int getOutcome(const game::config::HostConfiguration& config, const game::spec::ShipList& shipList, size_t slot);
        virtual Playability getPlayability(const game::config::HostConfiguration& config, const game::spec::ShipList& shipList);
        virtual void prepareResult(const game::config::HostConfiguration& config, const game::spec::ShipList& shipList, int resultLevel);
        virtual String_t getAlgorithmName(afl::string::Translator& tx) const;
        virtual bool isESBActive(const game::config::HostConfiguration& config) const;
        virtual afl::base::Optional<game::map::Point> getPosition() const;
        virtual afl::base::Optional<int32_t> getAuxiliaryInformation(AuxInfo info) const;
        virtual String_t getResultSummary(int viewpointPlayer, const game::config::HostConfiguration& config, const game::spec::ShipList& shipList, util::NumberFormatter fmt, afl::string::Translator& tx) const;
        virtual bool computeScores(Score& score, size_t slot, const game::config::HostConfiguration& config, const game::spec::ShipList& shipList) const;

     private:
        struct Info;
        afl::container::PtrVector<Info> m_infos;
        std::vector<GroupInfo> m_groups;
        Playability m_playability;
        String_t m_algorithmName;
        bool m_esbActive;
        afl::base::Optional<game::map::Point> m_position;
        util::Vector<afl::base::Optional<int32_t>, AuxInfo> m_auxInfo;
    };

} } }

#endif
