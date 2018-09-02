/**
  *  \file game/ref/configuration.hpp
  */
#ifndef C2NG_GAME_REF_CONFIGURATION_HPP
#define C2NG_GAME_REF_CONFIGURATION_HPP

#include "afl/base/deleter.hpp"
#include "game/session.hpp"

namespace game { namespace ref {

    class SortPredicate;

    const int ConfigSortById          = 0;
    const int ConfigSortByOwner       = 1;
    const int ConfigSortByHull        = 2;
    const int ConfigSortByMass        = 3;
    const int ConfigSortByFleet       = 4;
    const int ConfigSortByTowGroup    = 5;
    const int ConfigSortByBattleOrder = 6;
    const int ConfigSortByLocation    = 7;
    const int ConfigSortByHullMass    = 8;
    const int ConfigSortByDamage      = 9;
    const int ConfigSortByName        = 10;
    const int ConfigSortByNewPosition = 11;

    SortPredicate& createSortPredicate(int config, Session& session, afl::base::Deleter& del);


    struct Configuration {
        typedef std::pair<int, int> Order_t;

        Order_t order;
        // FIXME: favorites
    };

    void fetchConfiguration(Session& session, Configuration& config);
    void storeConfiguration(Session& session, const Configuration& config);

} }

#endif
