/**
  *  \file u/t_game_hostversion.cpp
  *  \brief Test for game::HostVersion
  */

#include "game/hostversion.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test formatting. */
void
TestGameHostVersion::testFormat()
{
    afl::string::NullTranslator tx;
    using game::HostVersion;

    // Unknown
    TS_ASSERT_EQUALS(HostVersion().toString(tx), "unknown");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Unknown, MKVERSION(3,0,0)).toString(tx), "unknown");

    // Tim-Host
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, 0).toString(tx), "Host");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,0,0)).toString(tx), "Host 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,16,1)).toString(tx), "Host 3.16.001");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,20,0)).toString(tx), "Host 3.20");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::Host, MKVERSION(3,22,27)).toString(tx), "Host 3.22.027");

    // PHost
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, 0).toString(tx), "PHost");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)).toString(tx), "PHost 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,16,1)).toString(tx), "PHost 3.16a");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,20,0)).toString(tx), "PHost 3.20");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,4,5)).toString(tx), "PHost 3.4e");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::PHost, MKVERSION(3,22,27)).toString(tx), "PHost 3.22.027");

    // SRace (Tim-Host variant)
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, 0).toString(tx), "SRace");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, MKVERSION(3,0,0)).toString(tx), "SRace 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::SRace, MKVERSION(3,16,1)).toString(tx), "SRace 3.16.001");

    // NuHost
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, 0).toString(tx), "NuHost");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, MKVERSION(3,0,0)).toString(tx), "NuHost 3.0");
    TS_ASSERT_EQUALS(HostVersion(HostVersion::NuHost, MKVERSION(3,16,1)).toString(tx), "NuHost 3.16.001");
}

/** Test accessors. */
void
TestGameHostVersion::testAccessor()
{
    game::HostVersion t;
    TS_ASSERT_EQUALS(t.getKind(), game::HostVersion::Unknown);
    TS_ASSERT_EQUALS(t.getVersion(), 0);

    t.set(game::HostVersion::PHost, MKVERSION(4,1,0));
    TS_ASSERT_EQUALS(t.getKind(), game::HostVersion::PHost);
    TS_ASSERT_EQUALS(t.getVersion(), MKVERSION(4,1,0));

    TS_ASSERT_EQUALS(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,0)).getKind(), game::HostVersion::Host);
}

/** Test MKVERSION.
    These values are given to scripts and therefore should be verified against known values. */
void
TestGameHostVersion::testVersion()
{
    TS_ASSERT_EQUALS(MKVERSION(0,0,0),   0);
    TS_ASSERT_EQUALS(MKVERSION(3,22,46), 322046);
    TS_ASSERT_EQUALS(MKVERSION(4,1,5),   401005);
}

