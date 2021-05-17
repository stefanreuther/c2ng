/**
  *  \file game/vcr/flak/configuration.cpp
  *  \brief Structure game::vcr::flak::Configuration
  */

#include "game/vcr/flak/configuration.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/configurationfileparser.hpp"
#include "util/string.hpp"

using afl::string::strCaseCompare;
using afl::string::Format;

namespace {
    const char* LOG_NAME = "game.vcr.flak";

    class FlakConfigurationParser : public util::ConfigurationFileParser {
     public:
        FlakConfigurationParser(game::vcr::flak::Configuration& config, afl::sys::LogListener& log, afl::string::Translator& tx)
            : ConfigurationFileParser(tx),
              m_config(config), m_log(log), m_translator(tx)
            { }

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& line);
        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message);
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line);

     private:
        game::vcr::flak::Configuration& m_config;
        afl::sys::LogListener& m_log;
        afl::string::Translator& m_translator;

        void assignNumber(const String_t& fileName, int lineNr, const String_t& value, int& out);
    };
}


void
FlakConfigurationParser::handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& /*line*/)
{
    // ex FlakConfigParser::assign
    if (strCaseCompare(name, "RatingBeamScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingBeamScale);
    } else if (strCaseCompare(name, "RatingTorpScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingTorpScale);
    } else if (strCaseCompare(name, "RatingBayScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingBayScale);
    } else if (strCaseCompare(name, "RatingMassScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingMassScale);
    } else if (strCaseCompare(name, "RatingPEBonus") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingPEBonus);
    } else if (strCaseCompare(name, "RatingFullAttackBonus") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingFullAttackBonus);
    } else if (strCaseCompare(name, "RatingRandomBonus") == 0) {
        assignNumber(fileName, lineNr, value, m_config.RatingRandomBonus);
    } else if (strCaseCompare(name, "StartingDistanceShip") == 0) {
        assignNumber(fileName, lineNr, value, m_config.StartingDistanceShip);
    } else if (strCaseCompare(name, "StartingDistancePlanet") == 0) {
        assignNumber(fileName, lineNr, value, m_config.StartingDistancePlanet);
    } else if (strCaseCompare(name, "StartingDistancePerPlayer") == 0) {
        assignNumber(fileName, lineNr, value, m_config.StartingDistancePerPlayer);
    } else if (strCaseCompare(name, "StartingDistancePerFleet") == 0) {
        assignNumber(fileName, lineNr, value, m_config.StartingDistancePerFleet);
    } else if (strCaseCompare(name, "CompensationShipScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationShipScale);
    } else if (strCaseCompare(name, "CompensationBeamScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationBeamScale);
    } else if (strCaseCompare(name, "CompensationTorpScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationTorpScale);
    } else if (strCaseCompare(name, "CompensationFighterScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationFighterScale);
    } else if (strCaseCompare(name, "CompensationLimit") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationLimit);
    } else if (strCaseCompare(name, "CompensationMass100KTScale") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationMass100KTScale);
    } else if (strCaseCompare(name, "CompensationAdjust") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CompensationAdjust);
    } else if (strCaseCompare(name, "CyborgDebrisRate") == 0) {
        assignNumber(fileName, lineNr, value, m_config.CyborgDebrisRate);
    } else if (strCaseCompare(name, "MaximumFleetSize") == 0) {
        assignNumber(fileName, lineNr, value, m_config.MaximumFleetSize);
    } else if (strCaseCompare(name, "SendUtilData") == 0) {
        if (util::stringMatch("Yes", value)) {
            m_config.SendUtilData = true;
        } else if (util::stringMatch("No", value)) {
            m_config.SendUtilData = false;
        } else {
            handleError(fileName, lineNr, m_translator("Invalid boolean setting"));
        }
    } else {
        handleError(fileName, lineNr, m_translator("Invalid keyword"));
    }
}

void
FlakConfigurationParser::handleError(const String_t& fileName, int lineNr, const String_t& message)
{
    // ex FlakConfigParser::error
    m_log.write(afl::sys::LogListener::Error, LOG_NAME, Format("%s:%d: %s", fileName, lineNr, message));
}

void
FlakConfigurationParser::handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
{ }

void
FlakConfigurationParser::assignNumber(const String_t& fileName, int lineNr, const String_t& value, int& out)
{
    // ex FlakConfigParser::assignNumber
    if (!afl::string::strToInteger(value, out)) {
        handleError(fileName, lineNr, m_translator("Invalid number"));
    }
}


/*
 *  Public Functions
 */

game::vcr::flak::Configuration::Configuration()
{
    initConfiguration(*this);
}

void
game::vcr::flak::initConfiguration(Configuration& config)
{
    // ex initFlakConfig
    config.RatingBeamScale            = 1;
    config.RatingTorpScale            = 1;
    config.RatingBayScale             = 8;
    config.RatingMassScale            = 1;
    config.RatingPEBonus              = 20;
    config.RatingFullAttackBonus      = 20;
    config.RatingRandomBonus          = 20;
    config.StartingDistanceShip       = 26000;
    config.StartingDistancePlanet     = 10000;
    config.StartingDistancePerPlayer  = 1000;
    config.StartingDistancePerFleet   = 5000;
    config.CompensationShipScale      = 0;
    config.CompensationBeamScale      = 30;
    config.CompensationTorpScale      = 90;
    config.CompensationFighterScale   = 90;
    config.CompensationLimit          = 500;
    config.CompensationMass100KTScale = 0;
    config.CompensationAdjust         = 0;
    config.SendUtilData               = true;
    config.CyborgDebrisRate           = 75;
    config.MaximumFleetSize           = 999;
}

void
game::vcr::flak::loadConfiguration(Configuration& config, afl::io::Stream& file, bool inSection, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex loadFlakConfig
    FlakConfigurationParser fp(config, log, tx);
    fp.setSection("flak", inSection);
    fp.parseFile(file);
}

void
game::vcr::flak::loadConfiguration(Configuration& config, afl::io::Directory& dir, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    // ex loadFlakConfig, loadPDKFlakConfig
    // Start with empty configuration
    initConfiguration(config);

    // Load from flak.src or pconfig.src
    afl::base::Ptr<afl::io::Stream> f = dir.openFileNT("flak.src", afl::io::FileSystem::OpenRead);
    bool inSection = true;
    if (f.get() == 0) {
        f = dir.openFileNT("pconfig.src", afl::io::FileSystem::OpenRead);
        inSection = false;
    }
    if (f.get() != 0) {
        log.write(log.Info, LOG_NAME, afl::string::Format(tx("Reading FLAK configuration from %s..."), f->getName()));
        loadConfiguration(config, *f, inSection, log, tx);
    }
}
