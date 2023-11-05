/**
  *  \file u/t_server_host_spec_publisherimpl.cpp
  *  \brief Test for server::host::spec::PublisherImpl
  */

#include "server/host/spec/publisherimpl.hpp"

#include "t_server_host_spec.hpp"
#include "afl/data/access.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/sys/log.hpp"
#include "game/test/files.hpp"
#include "server/file/internalfileserver.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::data::Access;

namespace {
    const char*const PATH_NAME = "sdir";

    struct Environment {
        afl::base::Ref<afl::io::InternalDirectory> defaultSpecDir;
        server::file::InternalFileServer hostFile;
        server::interface::FileBaseClient hostFileClient;
        afl::sys::Log log;

        server::host::spec::PublisherImpl testee;

        Environment()
            : defaultSpecDir(afl::io::InternalDirectory::create("default")),
              hostFile(),
              hostFileClient(hostFile),
              log(),
              testee(*defaultSpecDir, hostFile, log)
            {
                hostFileClient.createDirectoryTree(PATH_NAME);
            }
    };

    void addFilesToDefault(Environment& env)
    {
        env.defaultSpecDir->openFile("beamspec.dat", afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultBeams());
        env.defaultSpecDir->openFile("torpspec.dat", afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultTorpedoes());
        env.defaultSpecDir->openFile("engspec.dat",  afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultEngines());
        env.defaultSpecDir->openFile("hullspec.dat", afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultHulls());
        env.defaultSpecDir->openFile("truehull.dat", afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultHullAssignments());
        env.defaultSpecDir->openFile("race.nm",      afl::io::FileSystem::Create)->fullWrite(game::test::getDefaultRaceNames());
    }

    void addFilesToHost(Environment& env)
    {
        env.hostFileClient.putFile("sdir/beamspec.dat", afl::string::fromBytes(game::test::getDefaultBeams()));
        env.hostFileClient.putFile("sdir/torpspec.dat", afl::string::fromBytes(game::test::getDefaultTorpedoes()));
        env.hostFileClient.putFile("sdir/engspec.dat",  afl::string::fromBytes(game::test::getDefaultEngines()));
        env.hostFileClient.putFile("sdir/hullspec.dat", afl::string::fromBytes(game::test::getDefaultHulls()));
        env.hostFileClient.putFile("sdir/truehull.dat", afl::string::fromBytes(game::test::getDefaultHullAssignments()));
        env.hostFileClient.putFile("sdir/race.nm",      afl::string::fromBytes(game::test::getDefaultRaceNames()));
    }
}

/** Single object access: beamspec.
    Indexes are off-by-one, we do not report a zeroth element. */
void
TestServerHostSpecPublisherImpl::testBeams()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("beamspec"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Heavy Phaser");
}

/** Single object access: config.
    Must read configuration file from data files, and add default values. */
void
TestServerHostSpecPublisherImpl::testConfig()
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/pconfig.src.frag", "GameName = hoho\nBeamHitOdds = 20\n");

    afl::data::StringList_t keys;
    keys.push_back("config");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("config"));
    TS_ASSERT_EQUALS(a("GAMENAME").toString(), "hoho");
    TS_ASSERT_EQUALS(a("BEAMHITODDS")[0].toInteger(), 20);
    TS_ASSERT_EQUALS(a("STARBASECOST")[0]("MC").toInteger(), 900);
}

/** Single object access: engines.
    Indexes are off-by-one, we do not report a zeroth element. */
void
TestServerHostSpecPublisherImpl::testEngines()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("engspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("engspec"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "StarDrive 1");
    TS_ASSERT_EQUALS(a[8]("NAME").toString(), "Transwarp Drive");
}

/** Single object access: friendly codes. */
void
TestServerHostSpecPublisherImpl::testFCodes()
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/fcodes.cc", "bav,p,buy a vowel\n");

    afl::data::StringList_t keys;
    keys.push_back("fcodes");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("fcodes"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "bav");
    TS_ASSERT_EQUALS(a[0]("DESCRIPTION").toString(), "buy a vowel");
}

/** Single object access: FLAK configuration.
    Must read configuration file from data files, and add default values. */
void
TestServerHostSpecPublisherImpl::testFlakConfig()
{
    Environment env;
    addFilesToHost(env);

    // FLAK configuration for testing; deliberate case error to exercise that this is not just text pass-through
    env.hostFileClient.putFile("sdir/pconfig.src", "%flak\nRatingBeamSCALE = 3\n");

    afl::data::StringList_t keys;
    keys.push_back("flakconfig");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("flakconfig"));
    TS_ASSERT_EQUALS(a("CompensationBeamScale").toInteger(), 30);     // default
    TS_ASSERT_EQUALS(a("RatingBeamScale").toInteger(), 3);            // taken from config
}

/** Single object access: FLAK configuration, with FLAK tool.
    Must read configuration file from data files, and add default values. */
void
TestServerHostSpecPublisherImpl::testFlakConfigSeparate()
{
    Environment env;
    addFilesToHost(env);

    // FLAK configuration in separate directory
    env.hostFileClient.createDirectoryTree("fdir");
    env.hostFileClient.putFile("fdir/flak.src", "RatingBeamScale = 77\n");

    afl::data::StringList_t keys;
    keys.push_back("flakconfig");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "fdir", keys);
    Access a(result->get("flakconfig"));
    TS_ASSERT_EQUALS(a("CompensationBeamScale").toInteger(), 30);     // default
    TS_ASSERT_EQUALS(a("RatingBeamScale").toInteger(), 77);           // taken from config
}

/** Single object access: race names. */
void
TestServerHostSpecPublisherImpl::testRaceName()
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/pconfig.src.frag", "PlayerRace = 7,8,9,10\n");

    afl::data::StringList_t keys;
    keys.push_back("racename");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("racename"));
    TS_ASSERT_EQUALS(a[1]("RACE.ADJ").toString(), "Lizard");
    TS_ASSERT_EQUALS(a[1]("RACE.ID").toInteger(), 8);
}

/** Single object access: torpedoes.
    Indexes are off-by-one, we do not report a zeroth element. */
void
TestServerHostSpecPublisherImpl::testTorps()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("torpspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("torpspec"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Mark 8 Photon");
}

/** Single object access: hull mappings.
    Player indexes are off-by-one, we do not report a zeroth element. */
void
TestServerHostSpecPublisherImpl::testTruehull()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("truehull");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("truehull"));
    TS_ASSERT_EQUALS(a[0][0].toInteger(), 1);
    TS_ASSERT_EQUALS(a[0][10].toInteger(), 10);
    TS_ASSERT_EQUALS(a[10][0].toInteger(), 15);
}

/** Single object access: hull functions. */
void
TestServerHostSpecPublisherImpl::testHullfunc()
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/hullfunc.cc", "4,,TimeWarp\n");

    afl::data::StringList_t keys;
    keys.push_back("hullfunc");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("hullfunc"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "TimeWarp");
    TS_ASSERT_EQUALS(a[0]("ID").toInteger(), 4);
}

/** Single object access: all hulls.
    Indexes are off-by-one, we do not report a zeroth element. */
void
TestServerHostSpecPublisherImpl::testHulls()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("hullspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("hullspec"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(a[14]("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
    TS_ASSERT_EQUALS(a[104]("NAME").toString(), "MERLIN CLASS ALCHEMY SHIP");
}

/** Single object access: single hull. */
void
TestServerHostSpecPublisherImpl::testSingleHull()
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("hull15");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("hull15"));
    TS_ASSERT_EQUALS(a("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
    TS_ASSERT_EQUALS(a("CARGO.MAX").toInteger(), 70);
}

/** Single object access variation: files taken from default directory. */
void
TestServerHostSpecPublisherImpl::testFilesFromDefault()
{
    Environment env;
    addFilesToDefault(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access a(result->get("beamspec"));
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Heavy Phaser");
}

/** Multiple object access. */
void
TestServerHostSpecPublisherImpl::testMultiple()
{
    Environment env;
    addFilesToDefault(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    keys.push_back("torpspec");
    keys.push_back("hull15");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);

    TS_ASSERT_EQUALS(Access(result->get("beamspec"))[0]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(Access(result->get("torpspec"))[0]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(Access(result->get("hull15"))("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
}

/** Error case: no file.
    This causes the load operation to fail, producing no result.
    In production code, this case is probably not reachable due to preceding validity checks
    and availability of populated defaultSpecificationDirectory. */
void
TestServerHostSpecPublisherImpl::testErrorNoFile()
{
    Environment env;
    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    TS_ASSERT_THROWS(env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
}

/** Error case: bad keys. */
void
TestServerHostSpecPublisherImpl::testErrorBadKeys()
{
    Environment env;
    addFilesToDefault(env);

    // Genuine bad key
    {
        afl::data::StringList_t keys;
        keys.push_back("badkey");
        TS_ASSERT_THROWS(env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
    }

    // Prefix of a valid key
    // (Parser in buildValue() will originally accept it, but completeness check refuses it.)
    {
        afl::data::StringList_t keys;
        keys.push_back("hull15x");
        TS_ASSERT_THROWS(env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
    }

    // Nonexistant hull
    {
        afl::data::StringList_t keys;
        keys.push_back("hull150");
        TS_ASSERT_THROWS(env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
    }
}

