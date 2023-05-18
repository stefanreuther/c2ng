/**
  *  \file game/config/userconfiguration.hpp
  *  \brief Class game::config::UserConfiguration
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

    struct MarkerOptionDescriptor;

    /** User configuration (preferences).
        Represents the content of the pcc2.ini file.

        Note that the UserConfiguration is extended by other modules that index it with option descriptors
        unknown to us, to create additional configuration points.
        Placing it here is preferred, see below.
        The disadvantage is that knowledge about that other module's configuration needs to be here.

        In addition to actual user preferences, a UserConfiguration also contains information about a game directory,
        i.e. "this is the game directory for 'user @ planetscentral.com, game 75'".
        It is therefore used extensively during browsing. */
    class UserConfiguration : public Configuration {
     public:
        /** Number of canned markers that can be defined. */
        static const int NUM_CANNED_MARKERS = 10;

        /** Values for option ChartWheel. */
        enum WheelMode {
            WheelZoom,          ///< Zoom in/out. ex cw_Zoom
            WheelBrowse,        ///< Browse through list. ex cw_Tab
            WheelPage           ///< Page through objects. ex cw_Page
        };

        /** Constructor.
            Makes a default configuration. */
        UserConfiguration();

        /** Destructor. */
        ~UserConfiguration();

        /** Set default values. */
        void setDefaultValues();

        /** Load user configuration file (from profile).
            If the file exists, loads it; otherwise, does nothing.
            \param dir Profile directory
            \param log Logger
            \param tx  Translator */
        void loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Load game configuration file (from game directory).
            If the file exists, loads it; otherwise, does nothing.
            \param dir Game directory
            \param log Logger
            \param tx  Translator */
        void loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Save user configuration file (to profile).
            Updates the file with all options tagged as Source=User.
            \param dir Profile directory
            \param log Logger
            \param tx  Translator */
        void saveUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx) const;

        /** Save game configuration file (to game directory).
            Updates the file with all options tagged as Source=Game.
            \param dir Game directory
            \param log Logger
            \param tx  Translator */
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
        static const MarkerOptionDescriptor* getCannedMarker(int slot);


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

        // Messages
        static const StringOptionDescriptor  Messages_LastSearch;

        // Display
        static const IntegerOptionDescriptor Display_ThousandsSep;
        static const IntegerOptionDescriptor Display_Clans;
        static const IntegerOptionDescriptor Display_HullfuncImages;
        static const IntegerOptionDescriptor Tax_PredictRelative;
        static const IntegerOptionDescriptor Tax_PredictRatio;

        // Starchart
        // More chart options in game::map::Configuration, those are maintained by session setup and need not be here
        static const IntegerOptionDescriptor ChartAnimThreshold;
        static const IntegerOptionDescriptor ChartMouseStickiness;
        static const IntegerOptionDescriptor ChartScannerWarpWells;
        static const IntegerOptionDescriptor ChartWheel;
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
        static const IntegerOptionDescriptor Team_SyncTransfer;

        // Unpack
        static const IntegerOptionDescriptor Unpack_AcceptRaceNames;
        static const StringOptionDescriptor  Unpack_AttachmentTimestamp;
        static const IntegerOptionDescriptor Unpack_Format;
        static const IntegerOptionDescriptor Unpack_FixErrors;
        static const IntegerOptionDescriptor Unpack_TargetExt;
        static const int Unpack_Ask = 0, Unpack_Accept = 1, Unpack_Reject = 2;
        static const int UnpackFormat_DOS = 0, UnpackFormat_Windows = 1;

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

        // Task screen
        static const IntegerOptionDescriptor Task_PredictToEnd;
        static const IntegerOptionDescriptor Task_ShowDistances;

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
