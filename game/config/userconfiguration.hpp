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

    class UserConfiguration : public Configuration {
     public:
        UserConfiguration();
        ~UserConfiguration();

        void setDefaultValues();

        void loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);
        void loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);
        void saveGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx) const;

//     // Game-related options:
//     ConfigIntOption CollapseOldMessages;
//     ConfigIntOption RewrapMessages;
//     ConfigIntOption InstantBattleResult;

//     // Sound options
//     ConfigIntOption Sound16Bits;
//     ConfigIntOption SoundEnabled;
//     ConfigIntOption SoundFrequency;
//     ConfigIntOption SoundHeadphone;
//     ConfigIntOption SoundReverse;
//     ConfigIntOption SoundStereo;

//     // Starchart options
//     ConfigIntOption ChartAnimThreshold;
//     ConfigIntOption ChartMouseStickiness;
//     ConfigIntOption ChartScannerWarpWells;

//     // Export
//     ConfigStringOption ExportShipFields;
//     ConfigStringOption ExportPlanetFields;

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

        static const StringOptionDescriptor  Game_Charset;
        static const StringOptionDescriptor  Game_Type;
        static const StringOptionDescriptor  Game_User;
        static const StringOptionDescriptor  Game_Host;
        static const StringOptionDescriptor  Game_Id;
        static const IntegerOptionDescriptor Game_Finished;
        static const IntegerOptionDescriptor Game_ReadOnly;
        static const IntegerOptionDescriptor Game_AccessHostFiles;

        static const IntegerOptionDescriptor Display_ThousandsSep;
        static const IntegerOptionDescriptor Display_Clans;

        static const StringOptionDescriptor Backup_Result;

        static const IntegerOptionDescriptor Team_AutoSync;
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
