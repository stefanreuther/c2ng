/**
  *  \file client/dialogs/simulationflakratings.hpp
  *  \brief Simulator: FLAK Rating Editor
  */
#ifndef C2NG_CLIENT_DIALOGS_SIMULATIONFLAKRATINGS_HPP
#define C2NG_CLIENT_DIALOGS_SIMULATIONFLAKRATINGS_HPP

#include "afl/string/translator.hpp"
#include "ui/root.hpp"

namespace client { namespace dialogs {

    /** Data for FLAK rating editor. */
    struct SimulationFlakRatings {
        bool useDefaults;              ///< true to use defaults (fl_RatingOverride not set).
        int32_t flakRating;            ///< Rating value to use.
        int32_t defaultFlakRating;     ///< Default rating value used when useDefaults is set.
        int flakCompensation;          ///< Compensation value to use.
        int defaultFlakCompensation;   ///< Default compensation value used when useDefaults is set.

        SimulationFlakRatings()
            : useDefaults(false), flakRating(), defaultFlakRating(), flakCompensation(), defaultFlakCompensation()
            { }
    };

    /** Edit FLAK ratings.
        @param [in]     root    UI root
        @param [in,out] values  Values to edit
        @param [in]     tx      Translator
        @retval true  Dialog has been confirmed, `values` updated
        @retval false Dialog cancelled */
    bool editSimulationFlakRatings(ui::Root& root, SimulationFlakRatings& values, afl::string::Translator& tx);

} }

#endif
