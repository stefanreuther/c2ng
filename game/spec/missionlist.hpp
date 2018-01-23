/**
  *  \file game/spec/missionlist.hpp
  */
#ifndef C2NG_GAME_SPEC_MISSIONLIST_HPP
#define C2NG_GAME_SPEC_MISSIONLIST_HPP

#include <vector>
#include "afl/charset/charset.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/hostversion.hpp"
#include "game/spec/mission.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

    // /** Mission list. Contains a list of missions. There is a global instance
    //     as well the possibility to make subset lists. */
    class MissionList {
     public:
        typedef std::vector<Mission> Container_t;
        typedef Container_t::const_iterator Iterator_t;

        MissionList();
//     GMissionList(const GMissionList& other, GShip& ship);

        ~MissionList();

        /*
         *  Container interface
         */

        size_t size() const;

        Iterator_t begin() const;

        Iterator_t end() const;

        const Mission* at(size_t i) const;


        /*
         *  Manipulator interface
         */

        bool addMission(const Mission& msn);
        void sort();
        void clear();

        const Mission* getMissionByNumber(int id, PlayerSet_t raceMask) const;
        bool getIndexByNumber(int id, PlayerSet_t raceMask, size_t& index) const;

        void loadFromFile(afl::io::Stream& in, afl::sys::LogListener& log);
        void loadFromIniFile(afl::io::Stream& in, afl::charset::Charset& cs);

        /*
         *  Utilities
         */
        bool isMissionCloaking(int mission_id, int owner, const game::config::HostConfiguration& config, const HostVersion& host) const;

     private:
        Container_t m_data;
        uint32_t m_usedLetters;
    };

} }

#endif
