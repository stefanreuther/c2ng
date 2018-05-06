/**
  *  \file game/config/userconfiguration.cpp
  */

#include "game/config/userconfiguration.hpp"
#include "game/config/configurationparser.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/format.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "afl/io/textfile.hpp"

namespace {
    const char PCC2_INI[] = "pcc2.ini";
    const char LOG_NAME[] = "game.config.user";

    typedef afl::bits::SmallSet<game::config::ConfigurationOption::Source> Sources_t;

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

const game::config::StringOptionDescriptor  game::config::UserConfiguration::Game_Charset = {
    "Game.Charset"
};
const game::config::StringOptionDescriptor  game::config::UserConfiguration::Game_Type = {
    "Game.Type"
};
const game::config::StringOptionDescriptor  game::config::UserConfiguration::Game_User = {
    "Game.User"
};
const game::config::StringOptionDescriptor  game::config::UserConfiguration::Game_Host = {
    "Game.Host"
};
const game::config::StringOptionDescriptor  game::config::UserConfiguration::Game_Id = {
    "Game.Id"
};
const game::config::IntegerOptionDescriptor game::config::UserConfiguration::Game_Finished = {
    "Game.Finished",
    &BooleanValueParser::instance
};
const game::config::IntegerOptionDescriptor game::config::UserConfiguration::Game_ReadOnly = {
    "Game.ReadOnly",
    &BooleanValueParser::instance
};
const game::config::IntegerOptionDescriptor game::config::UserConfiguration::Game_AccessHostFiles = {
    "Game.AccessHostFiles",
    &BooleanValueParser::instance
};

const game::config::IntegerOptionDescriptor game::config::UserConfiguration::Display_ThousandsSep = {
    "Display.ThousandsSep",
    &BooleanValueParser::instance
};
const game::config::IntegerOptionDescriptor game::config::UserConfiguration::Display_Clans = {
    "Display.Clans",
    &BooleanValueParser::instance
};


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
    UserConfiguration& me = *this;
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

    me[Display_ThousandsSep].set(1);
    me[Display_Clans].set(0);

//     ExportShipFields.set("Id@5,Name@20");
//     ExportPlanetFields.set("Id@5,Name@20");
    markAllOptionsUnset();
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

String_t
game::config::UserConfiguration::formatNumber(int32_t n) const
{
    // ex numToString
    String_t result = afl::string::Format("%d", n);
    if ((*this)[Display_ThousandsSep]()) {
        // The limit is to avoid placing a thousands-separator as "-,234"
        size_t i = result.size();
        size_t limit = (i > 0 && (result[0] < '0' || result[0] > '9')) ? 4 : 3;
        while (i > limit) {
            i -= 3;
            result.insert(i, ",");
        }
    }
    return result;
}

String_t
game::config::UserConfiguration::formatPopulation(int32_t n) const
{
    // ex clansToString
    return ((*this)[Display_Clans]()
            ? formatNumber(n) + "c"
            : formatNumber(100*n));
}
