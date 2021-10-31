/**
  *  \file game/config/userconfiguration.hpp
  */
#ifndef C2NG_GAME_CONFIG_USERCONFIGURATION_HPP
#define C2NG_GAME_CONFIG_USERCONFIGURATION_HPP

#include "afl/base/inlineoptional.hpp"
#include "afl/io/directory.hpp"
#include "game/config/configuration.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/stringoption.hpp"
#include "util/profiledirectory.hpp"
#include "util/numberformatter.hpp"

namespace game { namespace config {

    class MarkerOption;

    class UserConfiguration : public Configuration {
     public:
        static const int NUM_CANNED_MARKERS = 10;

        UserConfiguration();
        ~UserConfiguration();

        void setDefaultValues();

        void loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);
        void loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);
        void saveGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Get game type.
            This returns the same value as UserConfiguration[Game_Type](), but does not create an empty option value if non exists.
            Use this function to access a UserConfiguration during loading to avoid modifying it.
            \return Game type; empty if none set */
        String_t getGameType() const;

        /** Get number formatter.
            \return NumberFormatter */
        util::NumberFormatter getNumberFormatter() const;

        /** Format a number.
            \param n Number
            \return Formatted number, using user's settings for Display_ThousandsSep. */
        String_t formatNumber(int32_t n) const;

        /** Format an optional number.
            \param value Number
            \return Formatted value, using user's settings for Display_ThousandsSep; empty if parameter was unset. */
        template<class StorageType, StorageType NullValue, class UserType>
        String_t formatNumber(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const;

        /** Format a number of clans.
            \param n Number
            \return Formatted number, using user's settings for Display_ThousandsSep, Display_Clans. */
        String_t formatPopulation(int32_t n) const;

        /** Format an optional number of clans.
            \param value Number
            \return Formatted value, using user's settings for Display_ThousandsSep, Display_Clans; empty if parameter was unset. */
        template<class StorageType, StorageType NullValue, class UserType>
        String_t formatPopulation(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const;

        /** Get canned marker configuration.
            \param slot Slot number, starting at 0
            \return Configuration option; null if slot number out of range */
        MarkerOption* getCannedMarker(int slot);


        /*
         *  Definition of options
         *
         *  Although it's possible to define options at the place of use, defining them here is preferred.
         *  This makes them appear in the list of known options (e.g. in configuration, "Pref" function, etc.),
         *  even when they have not yet been used in the current session.
         *  Defaults are set in setDefaultValues(), so there's no need to use IntegerOptionDescriptorWithDefault.
         */

        // Game options
        static const StringOptionDescriptor  Game_Charset;
        static const StringOptionDescriptor  Game_Type;
        static const StringOptionDescriptor  Game_User;
        static const StringOptionDescriptor  Game_Host;
        static const StringOptionDescriptor  Game_Id;
        static const IntegerOptionDescriptor Game_Finished;
        static const IntegerOptionDescriptor Game_ReadOnly;
        static const IntegerOptionDescriptor Game_AccessHostFiles;

        // Display
        static const IntegerOptionDescriptor Display_ThousandsSep;
        static const IntegerOptionDescriptor Display_Clans;
        static const IntegerOptionDescriptor Tax_PredictRelative;

        // Starchart
        // More chart options in game::map::Configuration, those are maintained by session setup and need not be here
        static const IntegerOptionDescriptor ChartScannerWarpWells;
        static const IntegerOptionDescriptor ChartRenderOptions[3][2];

        // Locking
        static const IntegerOptionDescriptor Lock_Left;
        static const IntegerOptionDescriptor Lock_Right;

        // Backup etc.
        static const StringOptionDescriptor  Backup_Chart;
        static const StringOptionDescriptor  Backup_Result;
        static const StringOptionDescriptor  Backup_Script;
        static const StringOptionDescriptor  Backup_Turn;
        static const StringOptionDescriptor  Backup_Util;
        static const StringOptionDescriptor  Maketurn_Target;

        // Team
        static const IntegerOptionDescriptor Team_AutoSync;

        // Unpack
        static const IntegerOptionDescriptor Unpack_AcceptRaceNames;
        static const StringOptionDescriptor  Unpack_AttachmentTimestamp;
        static const int Unpack_Ask = 0, Unpack_Accept = 1, Unpack_Reject = 2;

        // Export
        static const StringOptionDescriptor  ExportShipFields;
        static const StringOptionDescriptor  ExportPlanetFields;

        // Sorting
        static const IntegerOptionDescriptor Sort_History;
        static const IntegerOptionDescriptor Sort_Ship;
        static const IntegerOptionDescriptor Sort_Ship_Secondary;
        static const IntegerOptionDescriptor Sort_Cargo;
        static const IntegerOptionDescriptor Sort_Cargo_Secondary;
        static const IntegerOptionDescriptor Sort_Search;
        static const IntegerOptionDescriptor Sort_Search_Secondary;

        // Simulation
        static const IntegerOptionDescriptor Sim_NumThreads;
    };

} }


template<class StorageType, StorageType NullValue, class UserType>
String_t
game::config::UserConfiguration::formatNumber(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const
{
    UserType i;
    if (value.get(i)) {
        return formatNumber(i);
    } else {
        return String_t();
    }
}

template<class StorageType, StorageType NullValue, class UserType>
String_t
game::config::UserConfiguration::formatPopulation(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const
{
    UserType i;
    if (value.get(i)) {
        return formatPopulation(i);
    } else {
        return String_t();
    }
}

#endif
