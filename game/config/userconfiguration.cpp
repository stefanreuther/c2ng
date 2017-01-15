/**
  *  \file game/config/userconfiguration.cpp
  */

#include "game/config/userconfiguration.hpp"
#include "game/config/configurationparser.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"

namespace {
    const char PCC2_INI[] = "pcc2.ini";
    const char LOG_NAME[] = "game.config.user";
}

// FIXME: port these...
// ConfigStringOption opt_BackupTurn    (getUserPreferences(), "Backup.Turn");
// ConfigStringOption opt_BackupResult  (getUserPreferences(), "Backup.Result");
// ConfigStringOption opt_BackupUtil    (getUserPreferences(), "Backup.Util");
// ConfigStringOption opt_BackupChart   (getUserPreferences(), "Backup.Chart");
// ConfigStringOption opt_BackupScript  (getUserPreferences(), "Backup.Script");
// ConfigStringOption opt_UnpackSource  (getUserPreferences(), "Unpack.Source");    // FIXME: not implemented!
// ConfigStringOption opt_MaketurnTarget(getUserPreferences(), "Maketurn.Target");

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
//     : CollapseOldMessages(*this, "Messages.CollapseOld", ValueBoolParser::instance),
//       RewrapMessages(*this, "Messages.RewrapInbox", ValueBoolParser::instance),
//       InstantBattleResult(*this, "VCR.InstantResult", ValueBoolParser::instance),

//       Sound16Bits(*this, "Sound.16Bits", ValueBoolParser::instance),
//       SoundEnabled(*this, "Sound.Enabled", ValueBoolParser::instance),
//       SoundFrequency(*this, "Sound.Frequency", ValueIntParser::instance),
//       SoundHeadphone(*this, "Sound.Headphone", ValueBoolParser::instance),
//       SoundReverse(*this, "Sound.Reverse", ValueBoolParser::instance),
//       SoundStereo(*this, "Sound.Stereo", ValueBoolParser::instance),
//       ChartAnimThreshold(*this, "Chart.AnimThreshold", ValueIntParser::instance),
//       ChartMouseStickiness(*this, "Chart.MouseStickiness", ValueIntParser::instance),
//       ChartScannerWarpWells(*this, "Chart.Scanner.WarpWells", ValueBoolParser::instance),
//       TeamAutoSync(*this, "Team.AutoSync", ValueBoolParser::instance),
//       DisplayThousandSep(*this, "Display.ThousandsSep", ValueBoolParser::instance),
//       DisplayClans(*this, "Display.Clans", ValueBoolParser::instance),
//       ExportShipFields(*this, "Export.ShipFields"),
//       ExportPlanetFields(*this, "Export.PlanetFields")

//     CollapseOldMessages.set(false);
//     RewrapMessages.set(true);
//     InstantBattleResult.set(true);

//     Sound16Bits.set(true);
//     SoundEnabled.set(true);
//     SoundFrequency.set(22050);
//     SoundHeadphone.set(false);
//     SoundReverse.set(false);
//     SoundStereo.set(true);

//     ChartAnimThreshold.set(11);
//     ChartMouseStickiness.set(5);
//     ChartScannerWarpWells.set(0);

//     TeamAutoSync.set(1);

//     DisplayThousandSep.set(1);
//     DisplayClans.set(0);

//     ExportShipFields.set("Id@5,Name@20");
//     ExportPlanetFields.set("Id@5,Name@20");
}

void
game::config::UserConfiguration::loadUserConfiguration(util::ProfileDirectory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex game/pref.h:loadUserPreferences
    afl::base::Ptr<afl::io::Stream> stream = dir.openFileNT(PCC2_INI);
    if (stream.get() != 0) {
        // Parse the file
        log.write(log.Debug, LOG_NAME, afl::string::Format(tx.translateString("Reading configuration from %s...").c_str(), stream->getName()));
        ConfigurationParser parser(log, *this, ConfigurationOption::User);
        parser.setCharsetNew(new afl::charset::Utf8Charset());
        parser.parseFile(*stream);

        // Set all options to srcUser, no matter where they come from.
        // This will make sure the main config file always contains all options.
        // FIXME: port this
//         for (Config::iterator i = getUserPreferences().begin(); i != getUserPreferences().end(); ++i) {
//             i->setSource(ConfigOption::srcUser);
//         }

//         // Remember preferences version to write the file in the same place
//         loaded_pref_version = version;
    } else {
//         // No file found, so create it with the new version.
//         loaded_pref_version = 1;
    }
}

// /** Load directory preferences file on top of user preferences. */
void
game::config::UserConfiguration::loadGameConfiguration(afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex game/pref.h:loadDirectoryPreferences
    afl::base::Ptr<afl::io::Stream> stream = dir.openFileNT(PCC2_INI, afl::io::FileSystem::OpenRead);
    if (stream.get() != 0) {
        log.write(log.Debug, LOG_NAME, afl::string::Format(tx.translateString("Reading configuration from %s...").c_str(), stream->getName()));
        ConfigurationParser parser(log, *this, ConfigurationOption::Game);
        parser.setCharsetNew(new afl::charset::Utf8Charset());
        parser.parseFile(*stream);
    }
}
