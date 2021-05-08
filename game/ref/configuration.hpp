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
    const int ConfigSortByTransferTarget = 12;

    struct ConfigurationSelection;

    SortPredicate& createSortPredicate(int config, Session& session, afl::base::Deleter& del);
    SortPredicate& createSortPredicate(const ConfigurationSelection& sel, Session& session, afl::base::Deleter& del);

    struct Configuration {
        typedef std::pair<int, int> Order_t;

        Order_t order;
        // FIXME: favorites
    };

    extern const ConfigurationSelection REGULAR;
    extern const ConfigurationSelection CARGO_TRANSFER;
    extern const ConfigurationSelection SEARCH;

    void fetchConfiguration(Session& session, const ConfigurationSelection& sel, Configuration& config);
    void storeConfiguration(Session& session, const ConfigurationSelection& sel, const Configuration& config);

} }

#endif
