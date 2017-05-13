/**
  *  \file game/maint/difficultyrater.hpp
  */
#ifndef C2NG_GAME_MAINT_DIFFICULTYRATER_HPP
#define C2NG_GAME_MAINT_DIFFICULTYRATER_HPP

#include "afl/io/directory.hpp"
#include "game/config/collapsibleintegerarrayoption.hpp"
#include "game/limits.hpp"
#include "afl/bits/smallset.hpp"

namespace game { namespace maint {

    /** Rating game difficulty.
        This class computes game difficulty from shiplist, host and master configuration.
        Individual partial ratings can also be obtained (i.e. just a shiplist rating).

        Use the addXXX() functions to feed this function data.
        Any partial data can be used. */
    class DifficultyRater {
     public:
        enum Rating {
            ShiplistRating,
            MineralRating,
            NativeRating,
            ProductionRating
        };

        DifficultyRater();
        ~DifficultyRater();

        /** Process a ship list.
            \param dir Directory */
        void addShipList(afl::io::Directory& dir);

        /** Process a set of config files.
            \param dir Directory */
        void addConfigurationDirectory(afl::io::Directory& dir);

        /** Process a config file or fragment.
            \param s File */
        void addConfigurationFile(afl::io::Stream& s);

        /** Process a config option.
            \param name Option in format "section.key"
            \param value Value */
        void addConfigurationValue(String_t name, String_t value);

        /** Check whether a partial rating is known.
            \param which partial rating to query */
        bool isRatingKnown(Rating which) const;

        /** Get rating.
            \param which partial rating to query
            \return Rating [0.0, 1.0] */
        double getRating(Rating which) const;

        /** Get total game rating.
            \return Total rating [0.0, 1.0] */
        double getTotalRating() const;

     private:
        // Configuration
        static const int NUM_PLAYERS = 11;

        class OptionValue : public game::config::CollapsibleIntegerArrayOption<NUM_PLAYERS> {
         public:
            OptionValue();
        };

        // FIXME: this uses the original generic data structure.
        // It would make sense to convert to a more specific one, i.e. ranges with just 8 slots.
        enum Config {
            Master_CoreRangesUsual,
            Master_CoreRangesAlternate,
            Master_CoreUsualFrequency,
            Master_SurfaceRanges,

            Master_NativeFrequency,
            Master_NativeRanges,
            Master_NativeClansRange,

            Host_ProductionRate,
            Host_MiningRate,
            Host_ColonistTaxRate,
            Host_NativeTaxRate,
            Host_PlayerRace,
            Host_PlayerSpecialMission,
            Host_HissEffectRate
        };
        static const size_t Config_MAX = Host_HissEffectRate+1;

        afl::bits::SmallSet<Config> config_known;
        OptionValue config_values[Config_MAX];

        // Shiplist rating
        bool shiplist_known;
        int shiplist_average_cost;

        // Other ratings
        double getAverageMinerals() const;
        double getAverageNatives() const;
        double getAverageVPI() const;
    };

} }

#endif
