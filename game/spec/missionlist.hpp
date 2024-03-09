/**
  *  \file game/spec/missionlist.hpp
  *  \brief Class game::spec::MissionList
  */
#ifndef C2NG_GAME_SPEC_MISSIONLIST_HPP
#define C2NG_GAME_SPEC_MISSIONLIST_HPP

#include <vector>
#include <map>
#include "afl/base/optional.hpp"
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/spec/mission.hpp"
#include "util/stringlist.hpp"

namespace game { namespace spec {

    /** List of Starship Missions.
        Contains and owns a list of Mission objects.

        c2ng change: this class is not used any longer to make subset lists. */
    class MissionList {
     public:
        typedef std::vector<Mission> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        /** Grouped missions. */
        struct Grouped {
            /** Name of the "all missions" group. */
            String_t allName;

            /** List of missions, by group. */
            std::map<String_t, util::StringList> groups;
        };


        /** Constructor.
            Make empty list. */
        MissionList();

        /** Destructor. */
        ~MissionList();

        /*
         *  Container interface
         */

        /** Get number of missions.
            \return count */
        size_t size() const;

        /** Get iterator to beginning of list.
            \return iterator, valid as long as the list is not modified. */
        Iterator_t begin() const;

        /** Get iterator to one-past-end of list.
            \return iterator, valid as long as the list is not modified. */
        Iterator_t end() const;

        /** Get mission by index.
            \param i Index [0,size())
            \return Mission, is valid as long as the list is not modified; null if i is out of range */
        const Mission* at(size_t i) const;


        /*
         *  Manipulator interface
         */

        /** Add mission to list.
            This will add a (copy of) \c msn to the mission list.
            If the mission was already defined with a definition of equal or better quality, the call is ignored.
            \param msn Mission
            \return true if mission was added, false if call was ignored*/
        bool addMission(const Mission& msn);

        /** Sort mission list.
            Produces the conventional (numerical) order. */
        void sort();

        /** Clear mission list. */
        void clear();

        /** Find mission by number/player.
            \param id        Mission number
            \param raceMask  Mask of races (not players!) to match
            \return Mission if any found, otherwise null */
        const Mission* findMissionByNumber(int id, PlayerSet_t raceMask) const;

        /** Find index of a mission by number/player.
            \param id        Mission number
            \param raceMask  Mask of races (not players!) to match
            \return Index (for at()) if mission found; otherwise, Nothing */
        afl::base::Optional<size_t> findIndexByNumber(int id, PlayerSet_t raceMask) const;

        /** Load from mission.cc file.
            \param in   Stream
            \param log  Log listener
            \param tx   Translator */
        void loadFromFile(afl::io::Stream& in, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load from mission.ini file.
            \param in   Stream
            \param cs   Game character set */
        void loadFromIniFile(afl::io::Stream& in, afl::charset::Charset& cs);

        /** Get missions, separated into groups.
            \param [out] out   Result
            \param [in]  tx    Translator */
        void getGroupedMissions(Grouped& out, afl::string::Translator& tx) const;

        /*
         *  Utilities
         */

        /** Check whether a mission causes the ship to cloak.
            This applies to the Cloak and Super Spy missions as well as their extended counterparts.
            \param mission_id   Mission number
            \param owner        Ship owner
            \param config       Host configuration
            \return true if mission cloaks the ship */
        bool isMissionCloaking(int mission_id, int owner, const game::config::HostConfiguration& config) const;

        /** Check for extended mission.
            \param shipMission Mission number (from ship)
            \param checkFor    Extended mission number to check for (pmsn_XXX)
            \param config      Host configuration
            \return true on match */
        bool isExtendedMission(int shipMission, int checkFor, const game::config::HostConfiguration& config) const;

        /** Check for special mission.
            \param shipMission Mission number (from ship)
            \param config      Host configuration
            \return true if shipMission is either the special mission (msn_Special), or the special extended mission (pmsn_Special). */
        bool isSpecialMission(int shipMission, const game::config::HostConfiguration& config) const;

     private:
        Container_t m_data;
        uint32_t m_usedLetters;
    };

} }

#endif
