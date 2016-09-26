/**
  *  \file game/v3/rootloader.cpp
  */

#include "game/v3/rootloader.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileformatexception.hpp"
#include "afl/io/multidirectory.hpp"
#include "afl/string/format.hpp"
#include "game/config/configurationparser.hpp"
#include "game/v3/registrationkey.hpp"
#include "game/v3/specificationloader.hpp"
#include "game/v3/resultloader.hpp"
#include "game/v3/stringverifier.hpp"

namespace gt = game::v3::structures;
using afl::io::FileSystem;
using game::config::ConfigurationOption;

namespace {
    const int DEFAULT_PHOST_VERSION = MKVERSION(4,1,0);
    const int DEFAULT_HOST_VERSION = MKVERSION(3,22,26);

    const char LOG_NAME[] = "game.v3.rootloader";

    /** Import 11 WORDs from HCONFIG image (per-player settings). */
    void importArray16(game::config::HostConfiguration::StandardOption_t& option, gt::Int16_t (&image)[12])
    {
        for (int i = 1; i <= 11; ++i) {
            option.set(i, image[i]);
        }
    }

    /** Import 8 DWORDs from HCONFIG image (meteor settings). */
    void importArray32(game::config::IntegerArrayOption<8>& option, gt::Int32_t (&image)[8])
    {
        for (int i = 1; i <= 8; ++i) {
            option.set(i, image[i-1]);
        }
    }

}

game::v3::RootLoader::RootLoader(afl::base::Ptr<afl::io::Directory> defaultSpecificationDirectory,
                                 util::ProfileDirectory& profile,
                                 afl::string::Translator& tx,
                                 afl::sys::LogListener& log,
                                 afl::io::FileSystem& fs)
    : m_defaultSpecificationDirectory(defaultSpecificationDirectory),
      m_profile(profile),
      m_translator(tx),
      m_log(log),
      m_fileSystem(fs),
      m_scanner(*m_defaultSpecificationDirectory, tx, log),
      m_charset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1))
{ }

void
game::v3::RootLoader::setCharsetNew(afl::charset::Charset* p)
{
    if (p) {
        m_charset.reset(p);
    }
}

afl::base::Ptr<game::Root>
game::v3::RootLoader::load(afl::base::Ptr<afl::io::Directory> gameDirectory, bool forceEmpty)
{
    m_scanner.clear();
    m_scanner.scan(*gameDirectory, *m_charset);

    afl::base::Ptr<Root> result;
    if (!m_scanner.getDirectoryFlags().empty() || forceEmpty) {
        // Specification directory
        afl::base::Ptr<afl::io::MultiDirectory> spec = afl::io::MultiDirectory::create();
        spec->addDirectory(gameDirectory);
        spec->addDirectory(m_defaultSpecificationDirectory);

        // Registration key
        std::auto_ptr<RegistrationKey> key(new RegistrationKey(*m_charset));
        key->initFromDirectory(*gameDirectory, m_log);

        // Specification loader
        afl::base::Ptr<SpecificationLoader> specLoader(new SpecificationLoader(*m_charset, m_translator, m_log));

        // Produce result
        result = new Root(spec, gameDirectory, specLoader,
                          m_scanner.getDirectoryHostVersion(),
                          std::auto_ptr<game::RegistrationKey>(key),
                          std::auto_ptr<game::StringVerifier>(new StringVerifier(std::auto_ptr<afl::charset::Charset>(m_charset->clone()))));

        // Configuration
        loadConfiguration(*result);
        loadRaceNames(result->playerList(), *spec);

        // Preferences
        result->userConfiguration().loadUserConfiguration(m_profile, m_log, m_translator);
        result->userConfiguration().loadGameConfiguration(*gameDirectory, m_log, m_translator);

        // Turn loader
        if (m_scanner.getDirectoryFlags().contains(DirectoryScanner::HaveResult)) {
            result->setTurnLoader(new ResultLoader(*m_charset, m_translator, m_log, m_scanner, m_fileSystem));
        }
    }
    return result;
}

void
game::v3::RootLoader::loadConfiguration(Root& root)
{
    // ex game/config.cc:initConfig
    game::config::HostConfiguration& config = root.hostConfiguration();
    config.setDefaultValues();

    // Check pconfig.src
    // FIXME: do we really want to load these from specificationDirectory()?
    afl::base::Ptr<afl::io::Stream> file = root.specificationDirectory().openFileNT("pconfig.src", FileSystem::OpenRead);
    if (file.get() != 0) {
        // OK, PHost
        loadPConfig(root,
                    file,
                    root.specificationDirectory().openFileNT("shiplist.txt", FileSystem::OpenRead),
                    ConfigurationOption::Game);
    } else {
        // SRace
        file = root.gameDirectory().openFileNT("friday.dat", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadRaceMapping(root, *file, ConfigurationOption::Game);
        }

        // Regular host config
        file = root.specificationDirectory().openFileNT("hconfig.hst", FileSystem::OpenRead);
        if (file.get() != 0) {
            loadHConfig(root, *file, ConfigurationOption::Game);
        } else {
            m_log.write(m_log.Warn, LOG_NAME, m_translator.translateString("No host configuration file found, using defaults"));
        }
    }

    // Set Tim-Host defaults
    // FIXME: this belongs either in HostConfiguration, or in HostVersion
    HostVersion& host = root.hostVersion();
    if (host.getKind() != HostVersion::Host) {
        config[config.RoundGravityWells].set(1);
        config[config.CPEnableRemote].set(0);
    }
}

/** Load PCONFIG.SRC.
    \param pconf pconfig.src file
    \param shiplist shiplist.txt file, may be null. */
void
game::v3::RootLoader::loadPConfig(Root& root,
                                  afl::base::Ptr<afl::io::Stream> pconfig,
                                  afl::base::Ptr<afl::io::Stream> shiplist,
                                  game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadPConfig
    // Configure parser
    game::config::ConfigurationParser parser(m_log, root.hostConfiguration(), source);
    parser.setCharsetNew(m_charset->clone());

    // Load pconfig.src (mandatory)
    m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), pconfig->getName()));
    parser.setSection("phost", true);
    parser.parseFile(*pconfig);

    // Load shiplist.txt (optional)
    if (shiplist.get() != 0) {
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), shiplist->getName()));
        parser.setSection("phost", false);
        parser.parseFile(*shiplist);
    }

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::PHost, DEFAULT_PHOST_VERSION);
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
    }
}

void
game::v3::RootLoader::loadHConfig(Root& root,
                                  afl::io::Stream& hconfig,
                                  game::config::ConfigurationOption::Source source)
{
    // ex game/config.cc:loadHConfig, Config::assignFromHConfigImage
    // FIXME: do host version guessing in this function
    if (hconfig.getSize() > 10*sizeof(gt::HConfig)) {
        // FIXME: log only?
        throw afl::except::FileFormatException(hconfig, m_translator.translateString("File has invalid size"));
    }

    // Read hconfig
    m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Reading configuration from %s...").c_str(), hconfig.getName()));

    gt::HConfig image;
    size_t size = hconfig.read(afl::base::fromObject(image));

    // Assign values.
    // Instead of checking each option's position, we only check known version boundaries.
    game::config::HostConfiguration& config = root.hostConfiguration();
    if (size >= 10) {
        config[config.RecycleRate].set(image.RecycleRate); config[config.RecycleRate].setSource(source);
        config[config.RandomMeteorRate].set(image.RandomMeteorRate); config[config.RandomMeteorRate].setSource(source);
        config[config.AllowMinefields].set(image.AllowMinefields); config[config.AllowMinefields].setSource(source);
        config[config.AllowAlchemy].set(image.AllowAlchemy); config[config.AllowAlchemy].setSource(source);
        config[config.DeleteOldMessages].set(image.DeleteOldMessages); config[config.DeleteOldMessages].setSource(source);
    }
    if (size >= 186) {
        config[config.DisablePasswords].set(image.DisablePasswords); config[config.DisablePasswords].setSource(source);
        importArray16(config[config.GroundKillFactor], image.GroundKillFactor); config[config.GroundKillFactor].setSource(source);
        importArray16(config[config.GroundDefenseFactor], image.GroundDefenseFactor); config[config.GroundDefenseFactor].setSource(source);
        importArray16(config[config.FreeFighters], image.FreeFighters); config[config.FreeFighters].setSource(source);
        importArray16(config[config.RaceMiningRate], image.RaceMiningRate); config[config.RaceMiningRate].setSource(source);
        importArray16(config[config.ColonistTaxRate], image.ColonistTaxRate); config[config.ColonistTaxRate].setSource(source);
        config[config.RebelsBuildFighters].set(image.RebelsBuildFighters); config[config.RebelsBuildFighters].setSource(source);
        config[config.ColoniesBuildFighters].set(image.ColoniesBuildFighters); config[config.ColoniesBuildFighters].setSource(source);
        config[config.RobotsBuildFighters].set(image.RobotsBuildFighters); config[config.RobotsBuildFighters].setSource(source);
        config[config.CloakFailureRate].set(image.CloakFailureRate); config[config.CloakFailureRate].setSource(source);
        config[config.RobCloakedShips].set(image.RobCloakedShips); config[config.RobCloakedShips].setSource(source);
        config[config.ScanRange].set(image.ScanRange); config[config.ScanRange].setSource(source);
        config[config.DarkSenseRange].set(image.DarkSenseRange); config[config.DarkSenseRange].setSource(source);
        config[config.AllowHiss].set(image.AllowHiss); config[config.AllowHiss].setSource(source);
        config[config.AllowRebelGroundAttack].set(image.AllowRebelGroundAttack); config[config.AllowRebelGroundAttack].setSource(source);
        config[config.AllowSuperRefit].set(image.AllowSuperRefit); config[config.AllowSuperRefit].setSource(source);
        config[config.AllowWebMines].set(image.AllowWebMines); config[config.AllowWebMines].setSource(source);
        config[config.CloakFuelBurn].set(image.CloakFuelBurn); config[config.CloakFuelBurn].setSource(source);
        config[config.SensorRange].set(image.SensorRange); config[config.SensorRange].setSource(source);
        config[config.AllowNewNatives].set(image.AllowNewNatives); config[config.AllowNewNatives].setSource(source);
        config[config.AllowPlanetAttacks].set(image.AllowPlanetAttacks); config[config.AllowPlanetAttacks].setSource(source);
        config[config.BorgAssimilationRate].set(image.BorgAssimilationRate); config[config.BorgAssimilationRate].setSource(source);
        config[config.WebMineDecayRate].set(image.WebMineDecayRate); config[config.WebMineDecayRate].setSource(source);
        config[config.MineDecayRate].set(image.MineDecayRate); config[config.MineDecayRate].setSource(source);
        config[config.MaximumMinefieldRadius].set(image.MaximumMinefieldRadius); config[config.MaximumMinefieldRadius].setSource(source);
        config[config.TransuraniumDecayRate].set(image.TransuraniumDecayRate); config[config.TransuraniumDecayRate].setSource(source);
        config[config.StructureDecayPerTurn].set(image.StructureDecayPerTurn); config[config.StructureDecayPerTurn].setSource(source);
        config[config.AllowEatingSupplies].set(image.AllowEatingSupplies); config[config.AllowEatingSupplies].setSource(source);
        config[config.AllowNoFuelMovement].set(image.AllowNoFuelMovement); config[config.AllowNoFuelMovement].setSource(source);
        config[config.MineHitOdds].set(image.MineHitOdds); config[config.MineHitOdds].setSource(source);
        config[config.WebMineHitOdds].set(image.WebMineHitOdds); config[config.WebMineHitOdds].setSource(source);
        config[config.MineScanRange].set(image.MineScanRange); config[config.MineScanRange].setSource(source);
        config[config.AllowMinesDestroyMines].set(image.AllowMinesDestroyMines); config[config.AllowMinesDestroyMines].setSource(source);
    }
    if (size >= 288) {
        config[config.AllowEngineShieldBonus].set(image.AllowEngineShieldBonus); config[config.AllowEngineShieldBonus].setSource(source);
        config[config.EngineShieldBonusRate].set(image.EngineShieldBonusRate); config[config.EngineShieldBonusRate].setSource(source);
        // FIXME: _ColonialFighterSweepRate
        config[config.AllowColoniesSweepWebs].set(image.AllowColoniesSweepWebs); config[config.AllowColoniesSweepWebs].setSource(source);
        config[config.MineSweepRate].set(image.MineSweepRate); config[config.MineSweepRate].setSource(source);
        config[config.WebMineSweepRate].set(image.WebMineSweepRate); config[config.WebMineSweepRate].setSource(source);
        config[config.HissEffectRate].set(image.HissEffectRate); config[config.HissEffectRate].setSource(source);
        config[config.RobFailureOdds].set(image.RobFailureOdds); config[config.RobFailureOdds].setSource(source);
        config[config.PlanetsAttackRebels].set(image.PlanetsAttackRebels); config[config.PlanetsAttackRebels].setSource(source);
        config[config.PlanetsAttackKlingons].set(image.PlanetsAttackKlingons); config[config.PlanetsAttackKlingons].setSource(source);
        config[config.MineSweepRange].set(image.MineSweepRange); config[config.MineSweepRange].setSource(source);
        config[config.WebMineSweepRange].set(image.WebMineSweepRange); config[config.WebMineSweepRange].setSource(source);
        config[config.AllowScienceMissions].set(image.AllowScienceMissions); config[config.AllowScienceMissions].setSource(source);
        config[config.MineHitOddsWhenCloakedX10].set(image.MineHitOddsWhenCloakedX10); config[config.MineHitOddsWhenCloakedX10].setSource(source);
        config[config.DamageLevelForCloakFail].set(image.DamageLevelForCloakFail); config[config.DamageLevelForCloakFail].setSource(source);
        config[config.AllowFedCombatBonus].set(image.AllowFedCombatBonus); config[config.AllowFedCombatBonus].setSource(source);
        config[config.MeteorShowerOdds].set(image.MeteorShowerOdds); config[config.MeteorShowerOdds].setSource(source);
        importArray32(config[config.MeteorShowerOreRanges], image.MeteorShowerOreRanges); config[config.MeteorShowerOreRanges].setSource(source);
        config[config.LargeMeteorsImpacting].set(image.LargeMeteorsImpacting); config[config.LargeMeteorsImpacting].setSource(source);
        importArray32(config[config.LargeMeteorOreRanges], image.LargeMeteorOreRanges); config[config.LargeMeteorOreRanges].setSource(source);
        config[config.AllowMeteorMessages].set(image.AllowMeteorMessages); config[config.AllowMeteorMessages].setSource(source);
    }
    if (size >= 298) {
        config[config.AllowOneEngineTowing].set(image.AllowOneEngineTowing); config[config.AllowOneEngineTowing].setSource(source);
        config[config.AllowHyperWarps].set(image.AllowHyperWarps); config[config.AllowHyperWarps].setSource(source);
        config[config.ClimateDeathRate].set(image.ClimateDeathRate); config[config.ClimateDeathRate].setSource(source);
        config[config.AllowGravityWells].set(image.AllowGravityWells); config[config.AllowGravityWells].setSource(source);
        config[config.CrystalsPreferDeserts].set(image.CrystalsPreferDeserts); config[config.CrystalsPreferDeserts].setSource(source);
    }
    if (size >= 302) {
        config[config.AllowMinesDestroyWebs].set(image.AllowMinesDestroyWebs); config[config.AllowMinesDestroyWebs].setSource(source);
        config[config.ClimateLimitsPopulation].set(image.ClimateLimitsPopulation); config[config.ClimateLimitsPopulation].setSource(source);
    }
    if (size >= 328) {
        config[config.MaxPlanetaryIncome].set(image.MaxPlanetaryIncome); config[config.MaxPlanetaryIncome].setSource(source);
        config[config.IonStormActivity].set(image.IonStormActivity); config[config.IonStormActivity].setSource(source);
        config[config.AllowChunneling].set(image.AllowChunneling); config[config.AllowChunneling].setSource(source);
        config[config.AllowDeluxeSuperSpy].set(image.AllowDeluxeSuperSpy); config[config.AllowDeluxeSuperSpy].setSource(source);
        config[config.IonStormsHideMines].set(image.IonStormsHideMines); config[config.IonStormsHideMines].setSource(source);
        config[config.AllowGloryDevice].set(image.AllowGloryDevice); config[config.AllowGloryDevice].setSource(source);
        config[config.AllowAntiCloakShips].set(image.AllowAntiCloakShips); config[config.AllowAntiCloakShips].setSource(source);
        config[config.AllowGamblingShips].set(image.AllowGamblingShips); config[config.AllowGamblingShips].setSource(source);
        config[config.AllowCloakedShipsAttack].set(image.AllowCloakedShipsAttack); config[config.AllowCloakedShipsAttack].setSource(source);
        config[config.AllowShipCloning].set(image.AllowShipCloning); config[config.AllowShipCloning].setSource(source);
        config[config.AllowBoardingParties].set(image.AllowBoardingParties); config[config.AllowBoardingParties].setSource(source);
        config[config.AllowImperialAssault].set(image.AllowImperialAssault); config[config.AllowImperialAssault].setSource(source);
    }
    if (size >= 336) {
        config[config.RamScoopFuelPerLY].set(image.RamScoopFuelPerLY); config[config.RamScoopFuelPerLY].setSource(source);
        config[config.AllowAdvancedRefinery].set(image.AllowAdvancedRefinery); config[config.AllowAdvancedRefinery].setSource(source);
        config[config.AllowBioscanners].set(image.AllowBioscanners); config[config.AllowBioscanners].setSource(source);
        config[config.HullTechNotSlowedByMines].set(image.HullTechNotSlowedByMines); config[config.HullTechNotSlowedByMines].setSource(source);
    }
    // FIXME: _LokiDecloaksBirds
    if (size >= 340) {
        config[config.AllowVPAFeatures].set(image.AllowVPAFeatures); config[config.AllowVPAFeatures].setSource(source);
    }

    // Postprocess
    root.hostConfiguration().setDependantOptions();

    // Update host version guess
    HostVersion& host = root.hostVersion();
    if (host.getKind() == HostVersion::Unknown) {
        host.set(HostVersion::Host, DEFAULT_HOST_VERSION);
        m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
    }
}

void
game::v3::RootLoader::loadRaceMapping(Root& root, afl::io::Stream& file, game::config::ConfigurationOption::Source source)
{
    gt::Int16_t mapping[gt::NUM_PLAYERS];
    if (file.read(afl::base::fromObject(mapping)) == sizeof(mapping)) {
        // Load configuration option
        game::config::HostConfiguration& config = root.hostConfiguration();
        for (size_t i = 1; i <= gt::NUM_PLAYERS; ++i) {
            config[config.PlayerRace].set(i, mapping[i-1]);
        }
        config[config.PlayerSpecialMission].copyFrom(config[config.PlayerRace]);
        config[config.PlayerRace].setSource(source);
        config[config.PlayerSpecialMission].setSource(source);

        // Update host version guess
        HostVersion& host = root.hostVersion();
        if (host.getKind() == HostVersion::Unknown) {
            host.set(HostVersion::SRace, DEFAULT_HOST_VERSION);
            m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Host version not known, assuming %s").c_str(), host.toString(m_translator)));
        }
    }
}

void
game::v3::RootLoader::loadRaceNames(PlayerList& list, afl::io::Directory& dir)
{
    // ex GRaceNameList::load
    list.clear();

    // Load the file
    afl::base::Ptr<afl::io::Stream> file = dir.openFile("race.nm", FileSystem::OpenRead);
    gt::RaceNames in;
    file->fullRead(afl::base::fromObject(in));
    for (size_t player = 0; player < gt::NUM_PLAYERS; ++player) {
        if (Player* out = list.create(player+1)) {
            out->setName(Player::ShortName,     m_charset->decode(afl::string::toMemory(in.shortNames[player])));
            out->setName(Player::LongName,      m_charset->decode(afl::string::toMemory(in.longNames[player])));
            out->setName(Player::AdjectiveName, m_charset->decode(afl::string::toMemory(in.adjectiveNames[player])));
            out->setOriginalNames();
        }
    }

    // Create aliens
    if (Player* aliens = list.create(gt::NUM_PLAYERS+1)) {
        aliens->initAlien();
    }
}

// FIXME: delete?
// /** Save race names to specified 'race.nm' file. */
// void
// GRaceNameList::save(Stream& s) const
// {
//     char buffer[RN_FILE_SIZE];
//     for(int i = 1; i <= 11; ++i) {
//         storeBasicStringN(&buffer[RN_FULL_ORIG  + 30*(i-1)], 30, convertUtf8ToGame(full_names[i]));
//         storeBasicStringN(&buffer[RN_SHORT_ORIG + 20*(i-1)], 20, convertUtf8ToGame(short_names[i]));
//         storeBasicStringN(&buffer[RN_ADJ_ORIG   + 12*(i-1)], 12, convertUtf8ToGame(adjectives[i]));
//     }
//     s.writeT(buffer, sizeof(buffer));
// }
