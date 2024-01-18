/**
  *  \file test/server/host/spec/publisherimpltest.cpp
  *  \brief Test for server::host::spec::PublisherImpl
  */

#include "server/host/spec/publisherimpl.hpp"

#include "afl/data/access.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
AFL_TEST("server.host.spec.PublisherImpl:beamspec", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("beamspec"));
    a.checkEqual("01", ap[0]("NAME").toString(), "Laser");
    a.checkEqual("02", ap[9]("NAME").toString(), "Heavy Phaser");
}

/** Single object access: config.
    Must read configuration file from data files, and add default values. */
AFL_TEST("server.host.spec.PublisherImpl:config", a)
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/pconfig.src.frag", "GameName = hoho\nBeamHitOdds = 20\n");

    afl::data::StringList_t keys;
    keys.push_back("config");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("config"));
    a.checkEqual("01", ap("GAMENAME").toString(), "hoho");
    a.checkEqual("02", ap("BEAMHITODDS")[0].toInteger(), 20);
    a.checkEqual("03", ap("STARBASECOST")[0]("MC").toInteger(), 900);
}

/** Single object access: engines.
    Indexes are off-by-one, we do not report a zeroth element. */
AFL_TEST("server.host.spec.PublisherImpl:engspec", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("engspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("engspec"));
    a.checkEqual("01", ap[0]("NAME").toString(), "StarDrive 1");
    a.checkEqual("02", ap[8]("NAME").toString(), "Transwarp Drive");
}

/** Single object access: friendly codes. */
AFL_TEST("server.host.spec.PublisherImpl:fcodes", a)
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/fcodes.cc", "bav,p,buy a vowel\n");

    afl::data::StringList_t keys;
    keys.push_back("fcodes");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("fcodes"));
    a.checkEqual("01", ap[0]("NAME").toString(), "bav");
    a.checkEqual("02", ap[0]("DESCRIPTION").toString(), "buy a vowel");
}

/** Single object access: FLAK configuration.
    Must read configuration file from data files, and add default values. */
AFL_TEST("server.host.spec.PublisherImpl:flakconfig", a)
{
    Environment env;
    addFilesToHost(env);

    // FLAK configuration for testing; deliberate case error to exercise that this is not just text pass-through
    env.hostFileClient.putFile("sdir/pconfig.src", "%flak\nRatingBeamSCALE = 3\n");

    afl::data::StringList_t keys;
    keys.push_back("flakconfig");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("flakconfig"));
    a.checkEqual("01", ap("CompensationBeamScale").toInteger(), 30);     // default
    a.checkEqual("02", ap("RatingBeamScale").toInteger(), 3);            // taken from config
}

/** Single object access: FLAK configuration, with FLAK tool.
    Must read configuration file from data files, and add default values. */
AFL_TEST("server.host.spec.PublisherImpl:flakconfig:partial", a)
{
    Environment env;
    addFilesToHost(env);

    // FLAK configuration in separate directory
    env.hostFileClient.createDirectoryTree("fdir");
    env.hostFileClient.putFile("fdir/flak.src", "RatingBeamScale = 77\n");

    afl::data::StringList_t keys;
    keys.push_back("flakconfig");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "fdir", keys);
    Access ap(result->get("flakconfig"));
    a.checkEqual("01", ap("CompensationBeamScale").toInteger(), 30);     // default
    a.checkEqual("02", ap("RatingBeamScale").toInteger(), 77);           // taken from config
}

/** Single object access: race names. */
AFL_TEST("server.host.spec.PublisherImpl:racename", a)
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/pconfig.src.frag", "PlayerRace = 7,8,9,10\n");

    afl::data::StringList_t keys;
    keys.push_back("racename");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("racename"));
    a.checkEqual("01", ap[1]("RACE.ADJ").toString(), "Lizard");
    a.checkEqual("02", ap[1]("RACE.ID").toInteger(), 8);
}

/** Single object access: torpedoes.
    Indexes are off-by-one, we do not report a zeroth element. */
AFL_TEST("server.host.spec.PublisherImpl:torpspec", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("torpspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("torpspec"));
    a.checkEqual("01", ap[0]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("02", ap[9]("NAME").toString(), "Mark 8 Photon");
}

/** Single object access: hull mappings.
    Player indexes are off-by-one, we do not report a zeroth element. */
AFL_TEST("server.host.spec.PublisherImpl:truehull", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("truehull");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("truehull"));
    a.checkEqual("01", ap[0][0].toInteger(), 1);
    a.checkEqual("02", ap[0][10].toInteger(), 10);
    a.checkEqual("03", ap[10][0].toInteger(), 15);
}

/** Single object access: hull functions. */
AFL_TEST("server.host.spec.PublisherImpl:hullfunc", a)
{
    Environment env;
    addFilesToHost(env);
    env.hostFileClient.putFile("sdir/hullfunc.cc", "4,,TimeWarp\n");

    afl::data::StringList_t keys;
    keys.push_back("hullfunc");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("hullfunc"));
    a.checkEqual("01", ap[0]("NAME").toString(), "TimeWarp");
    a.checkEqual("02", ap[0]("ID").toInteger(), 4);
}

/** Single object access: all hulls.
    Indexes are off-by-one, we do not report a zeroth element. */
AFL_TEST("server.host.spec.PublisherImpl:hullspec", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("hullspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("hullspec"));
    a.checkEqual("01", ap[0]("NAME").toString(), "OUTRIDER CLASS SCOUT");
    a.checkEqual("02", ap[14]("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
    a.checkEqual("03", ap[104]("NAME").toString(), "MERLIN CLASS ALCHEMY SHIP");
}

/** Single object access: single hull. */
AFL_TEST("server.host.spec.PublisherImpl:single-hull", a)
{
    Environment env;
    addFilesToHost(env);

    afl::data::StringList_t keys;
    keys.push_back("hull15");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("hull15"));
    a.checkEqual("01", ap("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
    a.checkEqual("02", ap("CARGO.MAX").toInteger(), 70);
}

/** Single object access variation: files taken from default directory. */
AFL_TEST("server.host.spec.PublisherImpl:default-files", a)
{
    Environment env;
    addFilesToDefault(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);
    Access ap(result->get("beamspec"));
    a.checkEqual("01", ap[0]("NAME").toString(), "Laser");
    a.checkEqual("02", ap[9]("NAME").toString(), "Heavy Phaser");
}

/** Multiple object access. */
AFL_TEST("server.host.spec.PublisherImpl:multiple", a)
{
    Environment env;
    addFilesToDefault(env);

    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    keys.push_back("torpspec");
    keys.push_back("hull15");
    afl::data::Hash::Ref_t result = env.testee.getSpecificationData(PATH_NAME, "", keys);

    a.checkEqual("01", Access(result->get("beamspec"))[0]("NAME").toString(), "Laser");
    a.checkEqual("02", Access(result->get("torpspec"))[0]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("03", Access(result->get("hull15"))("NAME").toString(), "SMALL DEEP SPACE FREIGHTER");
}

/** Error case: no file.
    This causes the load operation to fail, producing no result.
    In production code, this case is probably not reachable due to preceding validity checks
    and availability of populated defaultSpecificationDirectory. */
AFL_TEST("server.host.spec.PublisherImpl:error:no-file", a)
{
    Environment env;
    afl::data::StringList_t keys;
    keys.push_back("beamspec");
    AFL_CHECK_THROWS(a, env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
}

/** Error case: bad keys. */

// Genuine bad key
AFL_TEST("server.host.spec.PublisherImpl:error:bad-key", a)
{
    Environment env;
    addFilesToDefault(env);
    afl::data::StringList_t keys;
    keys.push_back("badkey");
    AFL_CHECK_THROWS(a, env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
}

// Prefix of a valid key
// (Parser in buildValue() will originally accept it, but completeness check refuses it.)
AFL_TEST("server.host.spec.PublisherImpl:error:key-prefix", a)
{
    Environment env;
    addFilesToDefault(env);
    afl::data::StringList_t keys;
    keys.push_back("hull15x");
    AFL_CHECK_THROWS(a, env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
}

// Nonexistant hull
AFL_TEST("server.host.spec.PublisherImpl:error:bad-hull", a)
{
    Environment env;
    addFilesToDefault(env);
    afl::data::StringList_t keys;
    keys.push_back("hull150");
    AFL_CHECK_THROWS(a, env.testee.getSpecificationData(PATH_NAME, "", keys), std::runtime_error);
}
