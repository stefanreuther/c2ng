/**
  *  \file game/spec/hullassignmentlist.hpp
  */
#ifndef C2NG_GAME_SPEC_HULLASSIGNMENTLIST_HPP
#define C2NG_GAME_SPEC_HULLASSIGNMENTLIST_HPP

#include <vector>
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

    class HullAssignmentList {
     public:
        enum Mode {
            PlayerIndexed,
            RaceIndexed
        };

        HullAssignmentList();

        ~HullAssignmentList();

        void clear();

        void setMode(Mode mode);

        void add(int player, int position, int hullNr);

        void clearPlayer(int player);

        int getIndexFromHull(const game::config::HostConfiguration& config, int player, int hullNr) const;

        int getHullFromIndex(const game::config::HostConfiguration& config, int player, int index) const;

        int getMaxIndex(const game::config::HostConfiguration& config, int player) const;

     private:
        Mode m_mode;
        std::vector<std::vector<int> > m_mapping;

        int mapPlayer(const game::config::HostConfiguration& config, int player) const;
    };

} }

#endif
