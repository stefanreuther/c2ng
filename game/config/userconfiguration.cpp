/**
  *  \file game/config/userconfiguration.cpp
  */

#include "game/config/userconfiguration.hpp"
#include "afl/base/countof.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "game/config/bitsetvalueparser.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "game/config/configurationparser.hpp"
#include "game/config/enumvalueparser.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/markeroption.hpp"
#include "game/map/renderoptions.hpp"

using game::map::RenderOptions;

namespace {
    const char PCC2_INI[] = "pcc2.ini";
    const char LOG_NAME[] = "game.config.user";

    typedef afl::bits::SmallSet<game::config::ConfigurationOption::Source> Sources_t;

    const game::config::MarkerOptionDescriptor MARKER_CONFIG[] = {
        { "Chart.Marker0", 2,  9, },
        { "Chart.Marker1", 0,  9, },
        { "Chart.Marker2", 1,  9, },
        { "Chart.Marker3", 2,  9, },
        { "Chart.Marker4", 3,  9, },
        { "Chart.Marker5", 4,  9, },
        { "Chart.Marker6", 5,  9, },
        { "Chart.Marker7", 6,  9, },
        { "Chart.Marker8", 2, 10, },
        { "Chart.Marker9", 0, 10, },
    };

    void saveConfiguration(afl::io::Stream& out, const game::config::Configuration& in, Sources_t sources)
    {
        // FIXME: this function should probably in a more generic place.
        // We are more aggressive overwriting config files than PCC2.
        // Whereas PCC2 only updates known keys, we load all keys, so we can rewrite the files.
        // However, this will lose comments and formatting.

        // Text file
        afl::io::TextFile tf(out);
        tf.setCharsetNew(new afl::charset::Utf8Charset());

        // Iterate
        afl::base::Ref<game::config::Configuration::Enumerator_t> opts = in.getOptions();
        game::config::Configuration::OptionInfo_t opt;
        while (opts->getNextElement(opt)) {
            if (opt.second != 0 && sources.contains(opt.second->getSource())) {
                tf.writeLine(opt.first + " = " + opt.second->toString());
            }
        }

        // Finish
        tf.flush();
    }
}

namespace game { namespace config {

    const int UserConfiguration::NUM_CANNED_MARKERS;

    // Game
    const StringOptionDescriptor  UserConfiguration::Game_Charset         = { "Game.Charset" };
    const StringOptionDescriptor  UserConfiguration::Game_Type            = { "Game.Type" };
    const StringOptionDescriptor  UserConfiguration::Game_User            = { "Game.User" };
    const StringOptionDescriptor  UserConfiguration::Game_Host            = { "Game.Host" };
    const StringOptionDescriptor  UserConfiguration::Game_Id              = { "Game.Id" };
    const IntegerOptionDescriptor UserConfiguration::Game_Finished        = { "Game.Finished", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Game_ReadOnly        = { "Game.ReadOnly", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Game_AccessHostFiles = { "Game.AccessHostFiles", &BooleanValueParser::instance };

    // Messages
    const StringOptionDescriptor  UserConfiguration::Messages_LastSearch  = { "Messages.LastSearch" };

    // Display
    const IntegerOptionDescriptor UserConfiguration::Display_ThousandsSep = { "Display.ThousandsSep", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Display_Clans        = { "Display.Clans", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Display_HullfuncImages = { "Display.HullfuncImages", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Tax_PredictRelative  = { "Tax.PredictRelative", &BooleanValueParser::instance };

    // Chart
    const IntegerOptionDescriptor UserConfiguration::ChartAnimThreshold    = { "Chart.AnimThreshold", &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::ChartMouseStickiness  = { "Chart.MouseStickiness", &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::ChartScannerWarpWells = { "Chart.Scanner.WarpWells", &BooleanValueParser::instance };

    // Order of options must agree with enum WheelMode in header
    namespace { EnumValueParser parse_chartwheel("zoom,browse,page"); }
    const IntegerOptionDescriptor UserConfiguration::ChartWheel = { "Chart.Wheel", &parse_chartwheel };

    // Order of bits must agree with enum RenderOptions::Option.
    // Order of options must agree with RenderOptions::Area.
    namespace { BitsetValueParser parse_chartopts("ion,mine,ufos,sectors,borders,drawings,selection,labels,trails,shipdots,warpwells,messages,decay"); }
    const IntegerOptionDescriptor UserConfiguration::ChartRenderOptions[3][2] = {
        // Small
        { { "Chart.Small.Show", &parse_chartopts, },
          { "Chart.Small.Fill", &parse_chartopts } },
        // Normal
        { { "Chart.Normal.Show", &parse_chartopts },
          { "Chart.Normal.Fill", &parse_chartopts } },
        // Scanner
        { { "Chart.Scanner.Show", &parse_chartopts },
          { "Chart.Scanner.Fill", &parse_chartopts } }
    };

    // Lock
    // Note that the order of bits must agree with the definitions of MatchPlanets etc. in game/map/locker.hpp.
    namespace { BitsetValueParser LockOptionParser("planet,ship,ufo,marker,minefield"); }
    const IntegerOptionDescriptor UserConfiguration::Lock_Left   = { "Lock.Left", &LockOptionParser };
    const IntegerOptionDescriptor UserConfiguration::Lock_Right  = { "Lock.Right", &LockOptionParser };

    // Backup etc.
    // ex opt_BackupResult, opt_BackupTurn, opt_MaketurnTarget
    const StringOptionDescriptor UserConfiguration::Backup_Chart    = { "Backup.Chart" };
    const StringOptionDescriptor UserConfiguration::Backup_Result   = { "Backup.Result" };
    const StringOptionDescriptor UserConfiguration::Backup_Script   = { "Backup.Script" };
    const StringOptionDescriptor UserConfiguration::Backup_Turn     = { "Backup.Turn" };
    const StringOptionDescriptor UserConfiguration::Backup_Util     = { "Backup.Util" };
    const StringOptionDescriptor UserConfiguration::Maketurn_Target = { "Maketurn.Target" };

    // Team
    const IntegerOptionDescriptor UserConfiguration::Team_AutoSync = { "Team.AutoSync", &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Team_SyncTransfer = { "Team.SyncTransfer", &BooleanValueParser::instance };  // @since 2.41

    // Unpack
    namespace { EnumValueParser Unpack_Parser("ask,accept,reject"); }
    const IntegerOptionDescriptor UserConfiguration::Unpack_AcceptRaceNames = { "Unpack.RaceNames", &Unpack_Parser };
    const StringOptionDescriptor UserConfiguration::Unpack_AttachmentTimestamp = { "Unpack.AttachmentTimestamp" };

    // Export
    const StringOptionDescriptor UserConfiguration::ExportShipFields   = { "Export.ShipFields" };
    const StringOptionDescriptor UserConfiguration::ExportPlanetFields = { "Export.PlanetFields" };

    // Sorting
    const IntegerOptionDescriptor UserConfiguration::Sort_History          = { "Sort.History",          &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Ship             = { "Sort.Ship",             &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Ship_Secondary   = { "Sort.Ship.Secondary",   &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Cargo            = { "Sort.Cargo",            &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Cargo_Secondary  = { "Sort.Cargo.Secondary",  &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Search           = { "Sort.Search",           &IntegerValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Sort_Search_Secondary = { "Sort.Search.Secondary", &IntegerValueParser::instance };

    // Task screen
    const IntegerOptionDescriptor UserConfiguration::Task_PredictToEnd  = { "Task.PredictToEnd",  &BooleanValueParser::instance };
    const IntegerOptionDescriptor UserConfiguration::Task_ShowDistances = { "Task.ShowDistances", &BooleanValueParser::instance };

    // Simulation
    const IntegerOptionDescriptor UserConfiguration::Sim_NumThreads        = { "Sim.NumThreads",        &IntegerValueParser::instance };
} }


/*
 *  UserConfiguration
 */

game::config::UserConfiguration::UserConfiguration()
    : Configuration()
{
    setDefaultValues();
}

game::config::UserConfiguration::~UserConfiguration()
{ }

void
game::config::UserConfiguration::setDefaultValues()
{
    // ex GUserPreferences::assignDefaults
    UserConfiguration& me = *this;

    // Unimplemented for now:
    //     : CollapseOldMessages(*this, "Messages.CollapseOld", ValueBoolParser::instance),
    //       RewrapMessages(*this, "Messages.RewrapInbox", ValueBoolParser::instance),
    //       InstantBattleResult(*this, "VCR.InstantResult", ValueBoolParser::instance),

    //       Sound16Bits(*this, "Sound.16Bits", ValueBoolParser::instance),
    //       SoundEnabled(*this, "Sound.Enabled", ValueBoolParser::instance),
    //       SoundFrequency(*this, "Sound.Frequency", ValueIntParser::instance),
    //       SoundHeadphone(*this, "Sound.Headphone", ValueBoolParser::instance),
    //       SoundReverse(*this, "Sound.Reverse", ValueBoolParser::instance),
    //       SoundStereo(*this, "Sound.Stereo", ValueBoolParser::instance),

    // ConfigStringOption opt_UnpackSource  (getUserPreferences(), "Unpack.Source");    // FIXME: not implemented in PCC2

    //     CollapseOldMessages.set(false);
    //     RewrapMessages.set(true);
    //     InstantBattleResult.set(true);

    //     Sound16Bits.set(true);
    //     SoundEnabled.set(true);
    //     SoundFrequency.set(22050);
    //     SoundHeadphone.set(false);
    //     SoundReverse.set(false);
    //     SoundStereo.set(true);

    // Game options are not for editing by user

    // Messages
    me[Messages_LastSearch].set(String_t());

    // Display
    me[Display_ThousandsSep].set(1);
    me[Display_Clans].set(0);
    me[Display_HullfuncImages].set(1);
    me[Tax_PredictRelative].set(0);

    // Starchart
    me[ChartAnimThreshold].set(11);
    me[ChartMouseStickiness].set(5);
    me[ChartScannerWarpWells].set(0);
    me[ChartWheel].set(0);
    me[ChartRenderOptions[RenderOptions::Small  ][0]].set(static_cast<int32_t>(RenderOptions::defaults().toInteger()));
    me[ChartRenderOptions[RenderOptions::Small  ][1]].set(static_cast<int32_t>((RenderOptions::defaults() & RenderOptions::tristate()).toInteger()));
    me[ChartRenderOptions[RenderOptions::Normal ][0]].set(static_cast<int32_t>(RenderOptions::defaults().toInteger()));
    me[ChartRenderOptions[RenderOptions::Normal ][1]].set(static_cast<int32_t>((RenderOptions::defaults() & RenderOptions::tristate()).toInteger()));
    me[ChartRenderOptions[RenderOptions::Scanner][0]].set(static_cast<int32_t>(RenderOptions::defaults().toInteger()));
    me[ChartRenderOptions[RenderOptions::Scanner][1]].set(static_cast<int32_t>((RenderOptions::defaults() & RenderOptions::tristate()).toInteger()));

    // Lock
    me[Lock_Left].set("planet,minefield,ufo");
    me[Lock_Right].set("ship,marker");

    // Backup
    me[Backup_Chart].set(String_t());
    me[Backup_Result].set(String_t());
    me[Backup_Script].set(String_t());
    me[Backup_Turn].set(String_t());
    me[Backup_Util].set(String_t());
    me[Maketurn_Target].set(String_t());

    // Team
    me[Team_AutoSync].set(1);
    me[Team_SyncTransfer].set(0);

    // Unpack
    me[Unpack_AcceptRaceNames].set(1);
    // Unpack_AttachmentTimestamp is not for editing by user

    // Export
    me[ExportShipFields].set("Id@5,Name@20");
    me[ExportPlanetFields].set("Id@5,Name@20");

    // Sorting
    me[Sort_History].set(0);
    me[Sort_Ship].set(0);
    me[Sort_Ship_Secondary].set(0);
    me[Sort_Cargo].set(12);               /* SortByTransferTarget */
    me[Sort_Cargo_Secondary].set(0);
    me[Sort_Search].set(0);
    me[Sort_Search_Secondary].set(0);

    // Marker
    for (size_t i = 0; i < countof(MARKER_CONFIG); ++i) {
        const MarkerOptionDescriptor& desc = MARKER_CONFIG[i];
        me[desc].set(MarkerOption::Data(desc.m_markerKind, desc.m_color, ""));
    }

    // Task screen
    me[Task_PredictToEnd].set(0);
    me[Task_ShowDistances].set(1);

    // Simulation
    me[Sim_NumThreads].set(0);

    markAllOptionsUnset();
}

void
game::config::UserConfiguration::loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex game/pref.h:loadUserPreferences
    afl::base::Ptr<afl::io::Stream> stream = dir.openFileNT(PCC2_INI);
    if (stream.get() != 0) {
        // Parse the file
        log.write(log.Debug, LOG_NAME, afl::string::Format(tx("Reading configuration from %s..."), stream->getName()));
        ConfigurationParser parser(log, tx, *this, ConfigurationOption::User);
        parser.setCharsetNew(new afl::charset::Utf8Charset());
        parser.parseFile(*stream);

        // Set all options to Source=User, no matter where they come from.
        // This will make sure the main config file always contains all (standard) options.
        setAllOptionsSource(ConfigurationOption::User);
    }
}

void
game::config::UserConfiguration::loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex game/pref.h:loadDirectoryPreferences
    afl::base::Ptr<afl::io::Stream> stream = dir.openFileNT(PCC2_INI, afl::io::FileSystem::OpenRead);
    if (stream.get() != 0) {
        log.write(log.Debug, LOG_NAME, afl::string::Format(tx("Reading configuration from %s..."), stream->getName()));
        ConfigurationParser parser(log, tx, *this, ConfigurationOption::Game);
        parser.setCharsetNew(new afl::charset::Utf8Charset());
        parser.parseFile(*stream);
    }
}

void
game::config::UserConfiguration::saveGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx) const
{
    afl::base::Ptr<afl::io::Stream> stream = dir.openFileNT(PCC2_INI, afl::io::FileSystem::Create);
    if (stream.get() != 0) {
        log.write(log.Debug, LOG_NAME, afl::string::Format(tx.translateString("Writing configuration to %s...").c_str(), stream->getName()));
        saveConfiguration(*stream, *this, Sources_t(ConfigurationOption::Game));
    }
}

String_t
game::config::UserConfiguration::getGameType() const
{
    if (const ConfigurationOption* p = getOptionByName(Game_Type.m_name)) {
        return p->toString();
    } else {
        return String_t();
    }
}

util::NumberFormatter
game::config::UserConfiguration::getNumberFormatter() const
{
    return util::NumberFormatter((*this)[Display_ThousandsSep](), (*this)[Display_Clans]());
}


String_t
game::config::UserConfiguration::formatNumber(int32_t n) const
{
    return getNumberFormatter().formatNumber(n);
}

String_t
game::config::UserConfiguration::formatPopulation(int32_t n) const
{
    return getNumberFormatter().formatPopulation(n);
}

const game::config::MarkerOptionDescriptor*
game::config::UserConfiguration::getCannedMarker(int slot)
{
    // ex ConfigMarkerOption::getCannedMarker
    static_assert(countof(MARKER_CONFIG) == NUM_CANNED_MARKERS, "NUM_CANNED_MARKERS");
    if (slot >= 0 && slot < static_cast<int>(countof(MARKER_CONFIG))) {
        return &MARKER_CONFIG[slot];
    } else {
        return 0;
    }
}
