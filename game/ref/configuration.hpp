/**
  *  \file game/ref/configuration.hpp
  *  \brief Reference List Configuration
  */
#ifndef C2NG_GAME_REF_CONFIGURATION_HPP
#define C2NG_GAME_REF_CONFIGURATION_HPP

#include "afl/base/deleter.hpp"
#include "game/session.hpp"

namespace game { namespace ref {

    class SortPredicate;

    /*
     *  Sort Orders
     */

    const int ConfigSortById          = 0;
    const int ConfigSortByOwner       = 1;
    const int ConfigSortByHull        = 2;
    const int ConfigSortByMass        = 3;
    const int ConfigSortByFleet       = 4;
    const int ConfigSortByTowGroup    = 5;
    const int ConfigSortByBattleOrder = 6;
    const int ConfigSortByPosition    = 7;
    const int ConfigSortByHullMass    = 8;
    const int ConfigSortByDamage      = 9;
    const int ConfigSortByName        = 10;
    const int ConfigSortByNextPosition = 11;
    const int ConfigSortByTransferTarget = 12;

    /** Configuration selection.
        Describes how sort predicate configuration is stored in the UserConfiguration. */
    struct ConfigurationSelection;

    /** ConfigurationSelection for usecase: regular object lists. */
    extern const ConfigurationSelection REGULAR;

    /** ConfigurationSelection for usecase: cargo transfer. */
    extern const ConfigurationSelection CARGO_TRANSFER;

    /** ConfigurationSelection for usecase: search. */
    extern const ConfigurationSelection SEARCH;

    /** Create sort predicate, given a sort order configuration.
        @param config  Configuration value (ConfigSortByXxx)
        @param session Session
        @param del     Deleter
        @return Sort predicate; owned by Deleter */
    const SortPredicate& createSortPredicate(int config, Session& session, afl::base::Deleter& del);

    /** Create sort predicate, given a ConfigurationSelection.
        Retrieves the configuration according to the given ConfigurationSelection, and constructs an appropriate sort predicate.
        @param sel     Configuration selection
        @param session Session
        @param del     Deleter
        @return Sort predicate; owned by Deleter */
    const SortPredicate& createSortPredicate(const ConfigurationSelection& sel, Session& session, afl::base::Deleter& del);


    /*
     *  Configuration
     */

    /** Sort order configuration. */
    struct Configuration {
        /** Sort order.
            First element is primary key, second element is secondary key.
            Each element is ConfigSortByXxxx. */
        typedef std::pair<int, int> Order_t;

        /** Sort order. */
        Order_t order;
    };

    /** Fetch configuration from session's user configuration.
        @param [in]  session  Session
        @param [in]  sel      ConfigurationSelection
        @param [out] config   Configuration */
    void fetchConfiguration(Session& session, const ConfigurationSelection& sel, Configuration& config);

    /** Store configuration in session's user configuration.
        @param [in]  session  Session
        @param [in]  sel      ConfigurationSelection
        @param [in]  config   Configuration */
    void storeConfiguration(Session& session, const ConfigurationSelection& sel, const Configuration& config);

} }

#endif
