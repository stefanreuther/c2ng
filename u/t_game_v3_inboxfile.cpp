/**
  *  \file u/t_game_v3_inboxfile.cpp
  *  \brief Test for game::v3::InboxFile
  */

#include "game/v3/inboxfile.hpp"

#include "t_game_v3.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test decodeMessage().
    A: provide messages affected by the known problems.
    E: verify that messages are decoded correctly. */
void
TestGameV3InboxFile::testDecodeMessage()
{
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    /* A message that is affected by Winplan's fixups (extra linefeeds).
       Dug up from a backup-of-a-backup-of-a-backup-of-a-backup of some sort
       (45ee08692bb2b085569bcc29a1df8d0595fcb3d0 /home/stefan/royale/archiv/00orig/rr/dos/l/old_d/turbo/vplanets/cc/games/s1plus/player2.rst) */
    static const uint8_t MESSAGE[] = {
        0x35, 0x3a, 0x7f, 0x44, 0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x2d, 0x60, 0x82, 0x6f, 0x2d, 0x60,
        0x7d, 0x6e, 0x70, 0x72, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x1a,
        0x53, 0x5f, 0x5c, 0x5a, 0x47, 0x2d, 0x61, 0x75, 0x72, 0x2d, 0x50, 0x7f, 0x86, 0x80, 0x81, 0x6e,
        0x79, 0x2d, 0x50, 0x7c, 0x7b, 0x73, 0x72, 0x71, 0x72, 0x7f, 0x6e, 0x81, 0x76, 0x7c, 0x7b, 0x1a,
        0x61, 0x5c, 0x47, 0x2d, 0x61, 0x75, 0x72, 0x2d, 0x59, 0x76, 0x87, 0x6e, 0x7f, 0x71, 0x2d, 0x4e,
        0x79, 0x79, 0x76, 0x6e, 0x7b, 0x70, 0x72, 0x1a, 0x1a, 0x49, 0x49, 0x49, 0x2d, 0x63, 0x5d, 0x4e,
        0x2d, 0x51, 0x6e, 0x81, 0x6e, 0x2d, 0x61, 0x7f, 0x6e, 0x7b, 0x80, 0x7a, 0x76, 0x80, 0x80, 0x76,
        0x7c, 0x7b, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x17, 0x1a, 0x17, 0x5c, 0x4f, 0x57, 0x52, 0x50, 0x61,
        0x47, 0x2d, 0x5d, 0x79, 0x6e, 0x7b, 0x72, 0x81, 0x2d, 0x3f, 0x45, 0x1a, 0x17, 0x51, 0x4e, 0x61,
        0x4e, 0x47, 0x2d, 0x3a, 0x3e, 0x42, 0x43, 0x42, 0x44, 0x3f, 0x3d, 0x41, 0x45, 0x42, 0x1a, 0x17,
        0x7c, 0x72, 0x6e, 0x6e, 0x7c, 0x72, 0x6e, 0x6e, 0x75, 0x6e, 0x6e, 0x6e, 0x75, 0x71, 0x77, 0x72,
        0x79, 0x71, 0x73, 0x6f, 0x6e, 0x6e, 0x73, 0x6f, 0x6e, 0x6e, 0x73, 0x6f, 0x6e, 0x6e, 0x78, 0x6f,
        0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x7a, 0x1a, 0x71, 0x1a, 0x17, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e,
        0x6e, 0x70, 0x6f, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x7a, 0x71, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e,
        0x6e, 0x73, 0x6f, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x79, 0x71, 0x6f, 0x6e, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x1a, 0x6e, 0x1a, 0x17, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x72, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x6e, 0x6e, 0x71, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6f, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x6e, 0x6e, 0x71, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x74, 0x1a, 0x73, 0x1a,
        0x17, 0x6e, 0x6e, 0x6f, 0x72, 0x6e, 0x6e, 0x74, 0x6f, 0x6e, 0x6e, 0x73, 0x71, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x72, 0x74, 0x6e, 0x6e, 0x72, 0x74, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x1a, 0x6e, 0x1a, 0x17, 0x6e, 0x6e, 0x6e, 0x6e,
        0x6e, 0x6e, 0x76, 0x72, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x6e, 0x75, 0x6e,
        0x6e, 0x6e, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x1a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x1a, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x1a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
        0x2d, 0x1a, 0x2d
    };
    static_assert(sizeof(MESSAGE) == 0x1F3, "sizeof MESSAGE");

    // Check with fixup enabled
    {
        const char EXPECTED[] =
            "(-r7000)<< Sub Space Message >>\n"
            "FROM: The Crystal Confederation\n"
            "TO: The Lizard Alliance\n"
            "\n"
            "<<< VPA Data Transmission >>>\n"
            "\n"
            "OBJECT: Planet 28\n"
            "DATA: -1565720485\n"
            "oeaaoeaahaaahdjeldfbaafbaafbaakbaaaaaamd\n"
            "aaaaaacbaaaaaamdaaaaaafbaaaaaaldbaaaaaaa\n"
            "aaaaaaeaaaaaaadaaaaaaabaaaaaaadaaaaaaagf\n"
            "aabeaagbaafdaaaaaaaaaaegaaegaaaaaaaaaaaa\n"
            "aaaaaaieaaaaaaaaaahaaa";
        static_assert(sizeof(EXPECTED) < sizeof(MESSAGE), "sizeof EXPECTED");
        TS_ASSERT_EQUALS(game::v3::decodeMessage(MESSAGE, cs, true), EXPECTED);
    }

    // Check with fixup disabled, for comparison
    {
        const char EXPECTED[] =
            "(-r7000)<< Sub Space Message >>\n"
            "FROM: The Crystal Confederation\n"
            "TO: The Lizard Alliance\n"
            "\n"
            "<<< VPA Data Transmission >>>\n"
            "\n"
            "OBJECT: Planet 28\n"
            "DATA: -1565720485\n"
            "oeaaoeaahaaahdjeldfbaafbaafbaakbaaaaaam\n"
            "d\n"
            "aaaaaacbaaaaaamdaaaaaafbaaaaaaldbaaaaaa\n"
            "a\n"
            "aaaaaaeaaaaaaadaaaaaaabaaaaaaadaaaaaaag\n"
            "f\n"
            "aabeaagbaafdaaaaaaaaaaegaaegaaaaaaaaaaa\n"
            "a\n"
            "aaaaaaieaaaaaaaaaahaaa";
        static_assert(sizeof(EXPECTED) < sizeof(MESSAGE), "sizeof EXPECTED");
        TS_ASSERT_EQUALS(game::v3::decodeMessage(MESSAGE, cs, false), EXPECTED);
    }

    /* A message with embedded nulls (race played with Dominate)
       2a3fc22668a24e5e8dd5c5b24236bb5498043533 /home/stefan/royale/root/home/stefan/pcc-v2/testgames/bird2/player5.rst */
    static const uint8_t NULL_MESSAGE[] = {
        0x35, 0x7c, 0x72, 0x3d, 0x40, 0x45, 0x42, 0x36, 0x49, 0x49, 0x49,
        0x2d, 0x56, 0x5c, 0x5b, 0x2d, 0x60, 0x81, 0x7c, 0x7f, 0x7a, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x53,
        0x7f, 0x7c, 0x7a, 0x47, 0x2d, 0x50, 0x7f, 0x86, 0x80, 0x79, 0x72, 0x7f, 0x0d, 0x0d, 0x82, 0x7a,
        0x0d, 0x6e, 0x81, 0x76, 0x7c, 0x7b, 0x0d, 0x0d, 0x0d, 0x1a, 0x60, 0x75, 0x76, 0x7d, 0x2d, 0x56,
        0x51, 0x2d, 0x5b, 0x82, 0x7a, 0x6f, 0x72, 0x7f, 0x47, 0x2d, 0x2d, 0x40, 0x45, 0x42, 0x1a, 0x5c,
        0x82, 0x7f, 0x2d, 0x80, 0x75, 0x76, 0x7d, 0x2d, 0x76, 0x80, 0x2d, 0x70, 0x6e, 0x82, 0x74, 0x75,
        0x81, 0x2d, 0x76, 0x7b, 0x1a, 0x56, 0x7c, 0x7b, 0x2d, 0x60, 0x81, 0x7c, 0x7f, 0x7a, 0x47, 0x2d,
        0x4f, 0x7f, 0x76, 0x71, 0x74, 0x72, 0x81, 0x1a, 0x56, 0x7c, 0x7b, 0x2d, 0x60, 0x81, 0x7c, 0x7f,
        0x7a, 0x2d, 0x56, 0x51, 0x2d, 0x30, 0x2d, 0x3e, 0x40, 0x1a, 0x64, 0x72, 0x2d, 0x75, 0x6e, 0x83,
        0x72, 0x2d, 0x6f, 0x72, 0x72, 0x7b, 0x2d, 0x7d, 0x82, 0x79, 0x79, 0x72, 0x71, 0x1a, 0x2d, 0x3f,
        0x44, 0x2d, 0x59, 0x66, 0x2d, 0x7c, 0x73, 0x73, 0x2d, 0x70, 0x7c, 0x82, 0x7f, 0x80, 0x72, 0x3b,
        0x1a, 0x51, 0x6e, 0x7a, 0x6e, 0x74, 0x72, 0x2d, 0x61, 0x6e, 0x78, 0x72, 0x7b, 0x47, 0x2d, 0x2d,
        0x44, 0x44, 0x1a, 0x50, 0x7f, 0x72, 0x84, 0x2d, 0x58, 0x76, 0x79, 0x79, 0x72, 0x71, 0x2d, 0x47,
        0x2d, 0x2d, 0x3d, 0x1a, 0x60, 0x75, 0x76, 0x72, 0x79, 0x71, 0x80, 0x2d, 0x6e, 0x7f, 0x72, 0x2d,
        0x71, 0x7c, 0x84, 0x7b, 0x3b,
    };
    static_assert(sizeof(NULL_MESSAGE) == 0xE0, "sizeof NULL_MESSAGE");

    {
        const char EXPECTED[] =
            "(oe0385)<<< ION Storm >>>\n"
            "From: Crysler\n"
            "Ship ID Number:  385\n"
            "Our ship is caught in\n"
            "Ion Storm: Bridget\n"
            "Ion Storm ID # 13\n"
            "We have been pulled\n"
            " 27 LY off course.\n"
            "Damage Taken:  77\n"
            "Crew Killed :  0\n"
            "Shields are down.";
        static_assert(sizeof(EXPECTED) < sizeof(NULL_MESSAGE), "sizeof EXPECTED");
        TS_ASSERT_EQUALS(game::v3::decodeMessage(NULL_MESSAGE, cs, true), EXPECTED);
    }
}

/** Test InboxFile class, happy path.
    A: construct InboxFile with a valid file.
    E: verify that messages are retrieved correctly. */
void
TestGameV3InboxFile::testInboxFile()
{
    /* Test data:
       a7884a2b283f521c2ed5460fc7b00dc9d83990ff /home/stefan/royale/repo/Backups/home/home/stefan/phost/mfq/testing/mdata6.dat */
    static const uint8_t DATA[] = {
        0x03, 0x00, 0x15, 0x00, 0x00, 0x00, 0x90, 0x00, 0xa5, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x45, 0x01,
        0x00, 0x00, 0xd6, 0x00, 0x35, 0x3a, 0x85, 0x3d, 0x3d, 0x3d, 0x3e, 0x36, 0x49, 0x49, 0x2d, 0x59,
        0x7c, 0x7b, 0x74, 0x2d, 0x5f, 0x6e, 0x7b, 0x74, 0x72, 0x2d, 0x60, 0x72, 0x7b, 0x80, 0x7c, 0x7f,
        0x80, 0x2d, 0x4b, 0x4b, 0x1a, 0x51, 0x76, 0x80, 0x81, 0x7f, 0x72, 0x80, 0x80, 0x2d, 0x70, 0x6e,
        0x79, 0x79, 0x2d, 0x6e, 0x7b, 0x71, 0x2d, 0x72, 0x85, 0x7d, 0x79, 0x7c, 0x80, 0x76, 0x7c, 0x7b,
        0x1a, 0x71, 0x72, 0x81, 0x72, 0x70, 0x81, 0x72, 0x71, 0x2d, 0x73, 0x7f, 0x7c, 0x7a, 0x2d, 0x6e,
        0x2d, 0x80, 0x81, 0x6e, 0x7f, 0x80, 0x75, 0x76, 0x7d, 0x2d, 0x6e, 0x81, 0x47, 0x1a, 0x35, 0x2d,
        0x3e, 0x42, 0x3d, 0x3d, 0x2d, 0x39, 0x2d, 0x3e, 0x46, 0x3d, 0x3d, 0x36, 0x1a, 0x56, 0x81, 0x2d,
        0x84, 0x6e, 0x80, 0x2d, 0x80, 0x75, 0x76, 0x7d, 0x2d, 0x30, 0x3e, 0x41, 0x40, 0x47, 0x1a, 0x5b,
        0x52, 0x62, 0x61, 0x5f, 0x5c, 0x5b, 0x56, 0x50, 0x2d, 0x53, 0x62, 0x52, 0x59, 0x2d, 0x50, 0x4e,
        0x5f, 0x5f, 0x56, 0x1a, 0x35, 0x3a, 0x75, 0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x49, 0x2d, 0x4e,
        0x70, 0x81, 0x76, 0x83, 0x76, 0x81, 0x86, 0x2d, 0x59, 0x72, 0x83, 0x72, 0x79, 0x2d, 0x5f, 0x72,
        0x7d, 0x7c, 0x7f, 0x81, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x1a, 0x5c, 0x79, 0x71, 0x2d, 0x4e, 0x70,
        0x81, 0x76, 0x83, 0x76, 0x81, 0x86, 0x2d, 0x59, 0x72, 0x83, 0x72, 0x79, 0x47, 0x2d, 0x2d, 0x2d,
        0x2d, 0x3d, 0x1a, 0x4e, 0x70, 0x81, 0x76, 0x83, 0x76, 0x81, 0x86, 0x2d, 0x59, 0x72, 0x83, 0x72,
        0x79, 0x2d, 0x51, 0x72, 0x70, 0x6e, 0x86, 0x47, 0x2d, 0x3a, 0x3d, 0x1a, 0x5b, 0x72, 0x84, 0x2d,
        0x4e, 0x70, 0x81, 0x76, 0x83, 0x76, 0x81, 0x86, 0x2d, 0x5d, 0x7c, 0x76, 0x7b, 0x81, 0x80, 0x47,
        0x2d, 0x2d, 0x38, 0x3d, 0x1a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a,
        0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x3a, 0x1a, 0x5b, 0x72, 0x84, 0x2d, 0x4e,
        0x70, 0x81, 0x76, 0x83, 0x76, 0x81, 0x86, 0x2d, 0x59, 0x72, 0x83, 0x72, 0x79, 0x47, 0x2d, 0x2d,
        0x2d, 0x2d, 0x3d, 0x1a, 0x35, 0x3a, 0x75, 0x3d, 0x3d, 0x3d, 0x36, 0x2d, 0x5d, 0x55, 0x5c, 0x60,
        0x61, 0x2d, 0x83, 0x40, 0x3b, 0x3f, 0x72, 0x1a, 0x55, 0x62, 0x59, 0x4a, 0x51, 0x53, 0x50, 0x41,
        0x3d, 0x50, 0x3d, 0x3f, 0x1a, 0x52, 0x5b, 0x54, 0x4a, 0x50, 0x46, 0x53, 0x53, 0x4e, 0x51, 0x51,
        0x44, 0x1a, 0x4f, 0x52, 0x4e, 0x4a, 0x4e, 0x40, 0x4f, 0x40, 0x40, 0x3f, 0x3f, 0x46, 0x1a, 0x61,
        0x5c, 0x5f, 0x4a, 0x46, 0x41, 0x42, 0x4e, 0x43, 0x44, 0x40, 0x3d, 0x1a, 0x61, 0x5f, 0x62, 0x4a,
        0x44, 0x41, 0x3d, 0x44, 0x3e, 0x45, 0x43, 0x3d, 0x1a, 0x5d, 0x65, 0x66, 0x4a, 0x46, 0x41, 0x44,
        0x44, 0x3e, 0x52, 0x4f, 0x45, 0x1a, 0x50, 0x53, 0x54, 0x4a, 0x53, 0x50, 0x40, 0x43, 0x42, 0x3f,
        0x50, 0x51, 0x1a, 0x5b, 0x4e, 0x5a, 0x4a, 0x46, 0x3e, 0x51, 0x53, 0x4e, 0x4f, 0x51, 0x40, 0x1a,
        0x1a, 0x61, 0x75, 0x72, 0x2d, 0x6e, 0x6f, 0x7c, 0x83, 0x72, 0x2d, 0x76, 0x7b, 0x73, 0x7c, 0x7f,
        0x7a, 0x6e, 0x81, 0x76, 0x7c, 0x7b, 0x2d, 0x76, 0x80, 0x2d, 0x73, 0x7c, 0x7f, 0x2d, 0x82, 0x80,
        0x72, 0x2d, 0x6f, 0x86, 0x1a, 0x72, 0x85, 0x81, 0x72, 0x7f, 0x7b, 0x6e, 0x79, 0x2d, 0x7d, 0x79,
        0x6e, 0x86, 0x72, 0x7f, 0x2d, 0x82, 0x81, 0x76, 0x79, 0x76, 0x81, 0x76, 0x72, 0x80, 0x2d, 0x6e,
        0x7b, 0x71, 0x2d, 0x70, 0x6e, 0x7b, 0x1a, 0x6f, 0x72, 0x2d, 0x80, 0x6e, 0x73, 0x72, 0x79, 0x86,
        0x2d, 0x76, 0x74, 0x7b, 0x7c, 0x7f, 0x72, 0x71, 0x3b, 0x1a
    };

    afl::string::NullTranslator tx;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::io::ConstMemoryStream ms(DATA);
    game::v3::InboxFile testee(ms, cs, tx);

    TS_ASSERT_EQUALS(testee.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(testee.loadMessage(0),
                     "(-x0001)<< Long Range Sensors >>\n"
                     "Distress call and explosion\n"
                     "detected from a starship at:\n"
                     "( 1500 , 1900)\n"
                     "It was ship #143:\n"
                     "NEUTRONIC FUEL CARRI");
    TS_ASSERT_EQUALS(testee.loadMessage(1),
                     "(-h000)<<< Activity Level Report >>>\n"
                     "\n"
                     "Old Activity Level:    0\n"
                     "Activity Level Decay: -0\n"
                     "New Activity Points:  +0\n"
                     "---------------------\n"
                     "New Activity Level:    0");
    TS_ASSERT_EQUALS(testee.loadMessage(2),
                     "(-h000) PHOST v3.2e\n"
                     "HUL=DFC40C02\n"
                     "ENG=C9FFADD7\n"
                     "BEA=A3B33229\n"
                     "TOR=945A6730\n"
                     "TRU=74071860\n"
                     "PXY=94771EB8\n"
                     "CFG=FC3652CD\n"
                     "NAM=91DFABD3\n"
                     "\n"
                     "The above information is for use by\n"
                     "external player utilities and can\n"
                     "be safely ignored.");
    TS_ASSERT_EQUALS(testee.loadMessage(3), "");
}

/** Test inbox file errors.
    A: prepare some bogus files.
    E: exception raised either during construction of InboxFile object (header parsing), or during message access.
    (It is not contractual which is which.) */
void
TestGameV3InboxFile::testInboxFileErrors()
{
    afl::string::NullTranslator tx;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    /*
     *  Header parsing
     */

    // Empty file
    {
        afl::io::ConstMemoryStream ms(afl::base::Nothing);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    // Negative count
    {
        static const uint8_t DATA[] = { 0xFF, 0xFE, 0, 0, 0 };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    // Truncated directory
    {
        static const uint8_t DATA[] = { 1,0, 9,0,0,0, 3 };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    // Bad size
    {
        static const uint8_t DATA[] = { 1,0, 9,0,0,0, 0xFF,0xFE };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    // Bad position
    {
        static const uint8_t DATA[] = { 1,0, 9,0,0,0x80, 0xFF,0xFE };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    // Bad position, too
    {
        static const uint8_t DATA[] = { 1,0, 0,0,0,0, 0xFF,0xFE };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).getNumMessages(), afl::except::FileProblemException);
    }

    /*
     *  Content parsing
     */

    // Truncated content
    {
        static const uint8_t DATA[] = { 1,0, 9,0,0,0, 3,0, 0x35,0x3A };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_THROWS(game::v3::InboxFile(ms, cs, tx).loadMessage(0), afl::except::FileProblemException);
    }

    // In contrast, this one is valid:
    {
        static const uint8_t DATA[] = { 1,0, 9,0,0,0, 3,0, 0x35,0x3A,0x36 };
        afl::io::ConstMemoryStream ms(DATA);
        TS_ASSERT_EQUALS(game::v3::InboxFile(ms, cs, tx).loadMessage(0), "(-)");
    }
}

/** Test header tweaking, "CC" header.
    A: prepare file with message containing a mangled "CC:" header.
    E: header decoded/unmangled correctly. */
void
TestGameV3InboxFile::testInboxFileTweakCC()
{
    /* Test data: part of
       55a5ca1b374f17e8401606be9a0d8b55ebfc094d  /home/stefan/royale/root/home/stefan/pcc-v2/testgames/htesting/player3.rst

       Original file has 51 messages; reduced 'count' in header to 1 because we only want the first one. */
    static const uint8_t DATA[] = {
        0x01, 0x00, 0x35, 0x01, 0x00, 0x00, 0x90, 0x00, 0xc5, 0x01, 0x00, 0x00, 0xe2, 0x00, 0xa7, 0x02,
        0x00, 0x00, 0xe4, 0x00, 0x8b, 0x03, 0x00, 0x00, 0xe2, 0x00, 0x6d, 0x04, 0x00, 0x00, 0xe4, 0x00,
        0x51, 0x05, 0x00, 0x00, 0xf1, 0x00, 0x42, 0x06, 0x00, 0x00, 0xf1, 0x00, 0x33, 0x07, 0x00, 0x00,
        0xe5, 0x00, 0x18, 0x08, 0x00, 0x00, 0xe2, 0x00, 0xfa, 0x08, 0x00, 0x00, 0xf9, 0x00, 0xf3, 0x09,
        0x00, 0x00, 0xc0, 0x00, 0xb3, 0x0a, 0x00, 0x00, 0xa7, 0x00, 0x5a, 0x0b, 0x00, 0x00, 0xa9, 0x00,
        0x03, 0x0c, 0x00, 0x00, 0x77, 0x00, 0x7a, 0x0c, 0x00, 0x00, 0x80, 0x00, 0xfa, 0x0c, 0x00, 0x00,
        0x7b, 0x00, 0x75, 0x0d, 0x00, 0x00, 0x7c, 0x00, 0xf1, 0x0d, 0x00, 0x00, 0x78, 0x00, 0x69, 0x0e,
        0x00, 0x00, 0x78, 0x00, 0xe1, 0x0e, 0x00, 0x00, 0x7c, 0x00, 0x5d, 0x0f, 0x00, 0x00, 0x7a, 0x00,
        0xd7, 0x0f, 0x00, 0x00, 0x7c, 0x00, 0x53, 0x10, 0x00, 0x00, 0x79, 0x00, 0xcc, 0x10, 0x00, 0x00,
        0x7a, 0x00, 0x46, 0x11, 0x00, 0x00, 0xc0, 0x00, 0x06, 0x12, 0x00, 0x00, 0x7b, 0x00, 0x81, 0x12,
        0x00, 0x00, 0xc1, 0x00, 0x42, 0x13, 0x00, 0x00, 0x7a, 0x00, 0xbc, 0x13, 0x00, 0x00, 0x7b, 0x00,
        0x37, 0x14, 0x00, 0x00, 0x79, 0x00, 0xb0, 0x14, 0x00, 0x00, 0x78, 0x00, 0x28, 0x15, 0x00, 0x00,
        0x83, 0x00, 0xab, 0x15, 0x00, 0x00, 0x7e, 0x00, 0x29, 0x16, 0x00, 0x00, 0x7d, 0x00, 0xa6, 0x16,
        0x00, 0x00, 0x76, 0x00, 0x1c, 0x17, 0x00, 0x00, 0x7d, 0x00, 0x99, 0x17, 0x00, 0x00, 0x76, 0x00,
        0x0f, 0x18, 0x00, 0x00, 0xbc, 0x00, 0xcb, 0x18, 0x00, 0x00, 0xb0, 0x00, 0x7b, 0x19, 0x00, 0x00,
        0x1c, 0x02, 0x97, 0x1b, 0x00, 0x00, 0x1c, 0x02, 0xb3, 0x1d, 0x00, 0x00, 0x24, 0x01, 0xd7, 0x1e,
        0x00, 0x00, 0x1e, 0x02, 0xf5, 0x20, 0x00, 0x00, 0x1e, 0x02, 0x13, 0x23, 0x00, 0x00, 0x1e, 0x02,
        0x31, 0x25, 0x00, 0x00, 0xaa, 0x00, 0xdb, 0x25, 0x00, 0x00, 0xbd, 0x00, 0x98, 0x26, 0x00, 0x00,
        0xc7, 0x01, 0x5f, 0x28, 0x00, 0x00, 0xdc, 0x00, 0x3b, 0x29, 0x00, 0x00, 0x78, 0x00, 0xb3, 0x29,
        0x00, 0x00, 0xd6, 0x00, 0x35, 0x3a, 0x7f, 0x40, 0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x49, 0x2d,
        0x60, 0x82, 0x6f, 0x2d, 0x60, 0x7d, 0x6e, 0x70, 0x72, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74,
        0x72, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x53, 0x5f, 0x5c, 0x5a, 0x47, 0x2d, 0x61, 0x75, 0x72, 0x2d,
        0x5f, 0x7c, 0x7a, 0x82, 0x79, 0x6e, 0x7b, 0x2d, 0x60, 0x81, 0x6e, 0x7f, 0x2d, 0x52, 0x7a, 0x7d,
        0x76, 0x7f, 0x72, 0x2d, 0x49, 0x40, 0x4b, 0x1a, 0x61, 0x5c, 0x2d, 0x2d, 0x47, 0x2d, 0x61, 0x75,
        0x72, 0x2d, 0x5f, 0x7c, 0x7a, 0x82, 0x79, 0x6e, 0x7b, 0x2d, 0x60, 0x81, 0x6e, 0x7f, 0x2d, 0x52,
        0x7a, 0x7d, 0x76, 0x7f, 0x72, 0x1a, 0x49, 0x50, 0x50, 0x47, 0x2d, 0x3e, 0x2d, 0x3f, 0x2d, 0x41,
        0x1a, 0x79, 0x72, 0x81, 0x34, 0x80, 0x2d, 0x81, 0x72, 0x6e, 0x7a, 0x2d, 0x82, 0x7d, 0x2d, 0x6e,
        0x74, 0x6e, 0x76, 0x7b, 0x80, 0x81, 0x2d, 0x81, 0x75, 0x72, 0x2d, 0x72, 0x7a, 0x7d, 0x76, 0x7f,
        0x72, 0x2e, 0x1a, 0x1a, 0x35, 0x3a, 0x80, 0x3d, 0x3f, 0x40, 0x3f, 0x36, 0x49, 0x49, 0x49, 0x2d,
        0x50, 0x79, 0x7c, 0x6e, 0x78, 0x2d, 0x53, 0x6e, 0x76, 0x79, 0x82, 0x7f, 0x72, 0x2e, 0x2d, 0x4b,
        0x4b, 0x4b, 0x1a, 0x1a, 0x5c, 0x82, 0x7f, 0x2d, 0x80, 0x75, 0x76, 0x7d, 0x2d, 0x5d, 0x50, 0x61,
        0x2d, 0x5d, 0x55, 0x5c, 0x52, 0x5b, 0x56, 0x65, 0x2d, 0x35, 0x30, 0x3f, 0x40, 0x3f, 0x36, 0x1a,
        0x75, 0x6e, 0x80, 0x2d, 0x80, 0x82, 0x73, 0x73, 0x72, 0x7f, 0x72, 0x71, 0x2d, 0x6e, 0x2d, 0x70,
        0x79, 0x7c, 0x6e, 0x78, 0x76, 0x7b, 0x74, 0x2d, 0x71, 0x72, 0x83, 0x76, 0x70, 0x72, 0x2d, 0x73,
        0x6e, 0x76, 0x79, 0x82, 0x7f, 0x72, 0x3b, 0x1a, 0x64, 0x72, 0x2d, 0x6e, 0x7f, 0x72, 0x2d, 0x7b,
        0x7c, 0x2d, 0x79, 0x7c, 0x7b, 0x74, 0x72, 0x7f, 0x2d, 0x70, 0x79, 0x7c, 0x6e, 0x78, 0x72, 0x71,
        0x3b, 0x2d, 0x5c, 0x82, 0x7f, 0x2d, 0x72, 0x7b, 0x74, 0x76, 0x7b, 0x72, 0x72, 0x7f, 0x80, 0x1a,
        0x7c, 0x7b, 0x2d, 0x81, 0x75, 0x76, 0x80, 0x2d, 0x80, 0x75, 0x76, 0x7d, 0x2d, 0x7f, 0x72, 0x7d,
        0x7c, 0x7f, 0x81, 0x2d, 0x81, 0x75, 0x6e, 0x81, 0x2d, 0x84, 0x72, 0x2d, 0x71, 0x7c, 0x2d, 0x7b,
        0x7c, 0x81, 0x2d, 0x75, 0x6e, 0x83, 0x72, 0x1a, 0x80, 0x82, 0x73, 0x73, 0x76, 0x70, 0x76, 0x72,
        0x7b, 0x81, 0x2d, 0x73, 0x82, 0x72, 0x79, 0x2d, 0x81, 0x7c, 0x2d, 0x7d, 0x7c, 0x84, 0x72, 0x7f,
        0x2d, 0x81, 0x75, 0x72, 0x2d, 0x70, 0x79, 0x7c, 0x6e, 0x78, 0x76, 0x7b, 0x74, 0x1a, 0x71, 0x72,
    };

    afl::string::NullTranslator tx;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::io::ConstMemoryStream ms(DATA);
    game::v3::InboxFile testee(ms, cs, tx);

    TS_ASSERT_EQUALS(testee.getNumMessages(), 1U);
    TS_ASSERT_EQUALS(testee.loadMessage(0),
                     "(-r3000)<<< Sub Space Message >>>\n"
                     "FROM: The Romulan Star Empire <3>\n"
                     "TO  : The Romulan Star Empire\n"
                     "CC: 1 2 4\n"
                     "let\'s team up against the empire!");
}

/** Test header tweaking, "Universal" header.
    With HOST.EXE, universal messages look like "FROM, TO, newline, UNIVERSAL, message text".
    We wrap that to have the UNIVERSAL header appear before the newline.

    A: prepare file with message containing a Universal Message.
    E: header correctly re-arranged. */
void
TestGameV3InboxFile::testInboxFileTweakUniversal()
{
    /* Test data: part of
       e30482258c5791dd2c9a9da5c145ea0e1e5aea00  Manos 3, turn 48, player 6
       (also exercises character set decoding.)
       (the mentioned VCR is PCC1's.)

       Original file has 38 messages; reduced 'count' in header to 3. */
    static const uint8_t DATA[] = {
        0x03, 0x00, 0xe7, 0x00, 0x00, 0x00, 0x55, 0x00, 0x3c, 0x01, 0x00, 0x00, 0x44, 0x01, 0x80, 0x02,
        0x00, 0x00, 0x0d, 0x01, 0x8d, 0x03, 0x00, 0x00, 0x0f, 0x02, 0x9c, 0x05, 0x00, 0x00, 0xe0, 0x01,
        0x7c, 0x07, 0x00, 0x00, 0x9f, 0x01, 0x1b, 0x09, 0x00, 0x00, 0xb6, 0x01, 0xd1, 0x0a, 0x00, 0x00,
        0x5f, 0x01, 0x30, 0x0c, 0x00, 0x00, 0x78, 0x01, 0xa8, 0x0d, 0x00, 0x00, 0x4a, 0x01, 0xf2, 0x0e,
        0x00, 0x00, 0x72, 0x01, 0x64, 0x10, 0x00, 0x00, 0xd9, 0x00, 0x3d, 0x11, 0x00, 0x00, 0x5e, 0x01,
        0x9b, 0x12, 0x00, 0x00, 0x8b, 0x00, 0x26, 0x13, 0x00, 0x00, 0xc6, 0x00, 0xec, 0x13, 0x00, 0x00,
        0xc4, 0x00, 0xb0, 0x14, 0x00, 0x00, 0xca, 0x00, 0x7a, 0x15, 0x00, 0x00, 0xc6, 0x00, 0x40, 0x16,
        0x00, 0x00, 0xcc, 0x00, 0x0c, 0x17, 0x00, 0x00, 0xa4, 0x00, 0xb0, 0x17, 0x00, 0x00, 0x13, 0x01,
        0xc3, 0x18, 0x00, 0x00, 0xfb, 0x00, 0xbe, 0x19, 0x00, 0x00, 0x04, 0x01, 0xc2, 0x1a, 0x00, 0x00,
        0xf8, 0x00, 0xba, 0x1b, 0x00, 0x00, 0x86, 0x01, 0x40, 0x1d, 0x00, 0x00, 0xe2, 0x00, 0x22, 0x1e,
        0x00, 0x00, 0xe4, 0x00, 0x06, 0x1f, 0x00, 0x00, 0xe3, 0x00, 0xe9, 0x1f, 0x00, 0x00, 0xe2, 0x00,
        0xcb, 0x20, 0x00, 0x00, 0xe3, 0x00, 0xae, 0x21, 0x00, 0x00, 0xdf, 0x00, 0x8d, 0x22, 0x00, 0x00,
        0x79, 0x00, 0x06, 0x23, 0x00, 0x00, 0x7a, 0x00, 0x80, 0x23, 0x00, 0x00, 0x9c, 0x00, 0x1c, 0x24,
        0x00, 0x00, 0xa1, 0x00, 0xbd, 0x24, 0x00, 0x00, 0x9a, 0x00, 0x57, 0x25, 0x00, 0x00, 0xb7, 0x00,
        0x0e, 0x26, 0x00, 0x00, 0x2e, 0x01, 0x35, 0x3a, 0x75, 0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x49,
        0x2d, 0x60, 0x82, 0x6f, 0x2d, 0x60, 0x7d, 0x6e, 0x70, 0x72, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e,
        0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x53, 0x5f, 0x5c, 0x5a, 0x47, 0x2d, 0x55, 0x7c, 0x80,
        0x81, 0x1a, 0x61, 0x5c, 0x47, 0x2d, 0x52, 0x83, 0x72, 0x7f, 0x86, 0x6f, 0x7c, 0x71, 0x86, 0x1a,
        0x7b, 0x91, 0x70, 0x75, 0x80, 0x81, 0x72, 0x7f, 0x2d, 0x75, 0x7c, 0x80, 0x81, 0x47, 0x2d, 0x7a,
        0x7c, 0x2d, 0x3f, 0x41, 0x3b, 0x3e, 0x3b, 0x3f, 0x3d, 0x3d, 0x3d, 0x35, 0x3a, 0x7f, 0x43, 0x3d,
        0x3d, 0x3d, 0x36, 0x49, 0x49, 0x2d, 0x60, 0x82, 0x6f, 0x2d, 0x60, 0x7d, 0x6e, 0x70, 0x72, 0x2d,
        0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x1a, 0x53, 0x5f, 0x5c, 0x5a, 0x47,
        0x2d, 0x61, 0x75, 0x72, 0x2d, 0x50, 0x86, 0x6f, 0x7c, 0x7f, 0x74, 0x1a, 0x61, 0x5c, 0x47, 0x2d,
        0x61, 0x75, 0x72, 0x2d, 0x50, 0x86, 0x6f, 0x7c, 0x7f, 0x74, 0x1a, 0x1a, 0x2d, 0x2d, 0x49, 0x49,
        0x49, 0x2d, 0x62, 0x7b, 0x76, 0x83, 0x72, 0x7f, 0x80, 0x6e, 0x79, 0x2d, 0x5a, 0x72, 0x80, 0x80,
        0x6e, 0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x4b, 0x1a, 0x3a, 0x3a, 0x3a, 0x2d, 0x53, 0x7c, 0x7f, 0x84,
        0x6e, 0x7f, 0x71, 0x72, 0x71, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x3a, 0x3a,
        0x3a, 0x1a, 0x35, 0x3a, 0x7d, 0x3d, 0x46, 0x41, 0x40, 0x36, 0x49, 0x49, 0x49, 0x2d, 0x60, 0x82,
        0x6f, 0x2d, 0x60, 0x7d, 0x6e, 0x70, 0x72, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d,
        0x4b, 0x4b, 0x4b, 0x1a, 0x1a, 0x51, 0x7f, 0x72, 0x80, 0x71, 0x72, 0x7b, 0x3a, 0x59, 0xa1, 0x6f,
        0x81, 0x6e, 0x82, 0x1a, 0x55, 0x6e, 0x80, 0x2d, 0x6f, 0x72, 0x72, 0x7b, 0x2d, 0x81, 0x6e, 0x78,
        0x72, 0x7b, 0x2d, 0x7c, 0x83, 0x72, 0x7f, 0x2d, 0x6f, 0x86, 0x2d, 0x81, 0x75, 0x72, 0x1a, 0x52,
        0x7a, 0x7d, 0x76, 0x7f, 0x72, 0x2d, 0x80, 0x75, 0x76, 0x7d, 0x1a, 0x60, 0x5a, 0x4e, 0x59, 0x59,
        0x2d, 0x51, 0x52, 0x52, 0x5d, 0x2d, 0x60, 0x5d, 0x4e, 0x50, 0x52, 0x2d, 0x53, 0x5f, 0x52, 0x56,
        0x1a, 0x3a, 0x3a, 0x3a, 0x2d, 0x52, 0x7b, 0x71, 0x2d, 0x53, 0x7c, 0x7f, 0x84, 0x6e, 0x7f, 0x71,
        0x72, 0x71, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x3a, 0x3a, 0x3a, 0x1a, 0x1a,
        0x4e, 0x7f, 0x7f, 0x7f, 0x74, 0x2e, 0x2d, 0x64, 0x72, 0x7f, 0x2d, 0x75, 0x6e, 0x81, 0x2d, 0x71,
        0x72, 0x7b, 0x2d, 0x63, 0x50, 0x5f, 0x2d, 0x83, 0x72, 0x7f, 0x6f, 0x7f, 0x7c, 0x70, 0x75, 0x72,
        0x7b, 0x2e, 0x1a, 0x71, 0x72, 0x7f, 0x2d, 0x87, 0x72, 0x76, 0x74, 0x81, 0x2d, 0x71, 0x6e, 0x80,
        0x2d, 0x73, 0x6e, 0x79, 0x80, 0x70, 0x75, 0x2d, 0x7f, 0x82, 0x7a, 0x2d, 0x6e, 0x7b, 0x2e, 0x35,
        0x3a, 0x7f, 0x46, 0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x2d, 0x60, 0x82, 0x6f, 0x2d, 0x60, 0x7d,
        0x6e, 0x70, 0x72, 0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x1a, 0x53,
        0x5f, 0x5c, 0x5a, 0x47, 0x2d, 0x61, 0x75, 0x72, 0x2d, 0x5f, 0x7c, 0x6f, 0x7c, 0x81, 0x76, 0x70,
        0x2d, 0x56, 0x7a, 0x7d, 0x72, 0x7f, 0x76, 0x82, 0x7a, 0x1a, 0x61, 0x5c, 0x47, 0x2d, 0x61, 0x75,
        0x72, 0x2d, 0x50, 0x86, 0x6f, 0x7c, 0x7f, 0x74, 0x1a, 0x1a, 0x4e, 0x79, 0x79, 0x76, 0x72, 0x71,
        0x2d, 0x5d, 0x79, 0x6e, 0x7b, 0x72, 0x81, 0x80, 0x1a, 0x5d, 0x6e, 0x80, 0x80, 0x70, 0x7c, 0x71,
        0x72, 0x2d, 0x47, 0x2d, 0x3e, 0x42, 0x3f, 0x3d, 0x1a, 0x3f, 0x2d, 0x46, 0x2d, 0x42, 0x3e, 0x46,
        0x2d, 0x40, 0x43, 0x2d, 0x41, 0x41, 0x2d, 0x41, 0x44, 0x2d, 0x41, 0x45, 0x2d, 0x45, 0x44, 0x2d,
        0x3e, 0x3e, 0x42, 0x2d, 0x3e, 0x41, 0x42, 0x1a, 0x3e, 0x42, 0x41, 0x2d, 0x3e, 0x44, 0x3d, 0x2d,
        0x3e, 0x44, 0x3e, 0x2d, 0x3e, 0x44, 0x41, 0x2d, 0x3e, 0x46, 0x40, 0x2d, 0x3e, 0x46, 0x46, 0x2d,
        0x3f, 0x3e, 0x41, 0x2d, 0x3f, 0x3e, 0x45, 0x2d, 0x44, 0x3f, 0x45, 0x2d, 0x3f, 0x41, 0x45, 0x1a,
        0x3f, 0x42, 0x44, 0x2d, 0x3f, 0x43, 0x3e, 0x2d, 0x3f, 0x43, 0x46, 0x2d, 0x3f, 0x44, 0x44, 0x2d,
        0x3f, 0x44, 0x45, 0x2d, 0x3f, 0x45, 0x3d, 0x2d, 0x3f, 0x45, 0x41, 0x2d, 0x3f, 0x45, 0x43, 0x2d,
        0x3f, 0x46, 0x3f, 0x2d, 0x40, 0x3d, 0x42, 0x1a, 0x40, 0x3e, 0x42, 0x2d, 0x40, 0x3e, 0x46, 0x2d,
        0x40, 0x41, 0x3f, 0x2d, 0x40, 0x41, 0x41, 0x2d, 0x40, 0x42, 0x3f, 0x2d, 0x40, 0x42, 0x46, 0x2d,
        0x40, 0x45, 0x43, 0x2d, 0x40, 0x46, 0x40, 0x2d, 0x40, 0x46, 0x41, 0x2d, 0x41, 0x42, 0x42, 0x1a,
        0x41, 0x43, 0x3e, 0x2d, 0x41, 0x44, 0x3d, 0x2d, 0x41, 0x44, 0x3e, 0x1a, 0x35, 0x3a, 0x7f, 0x4e,
        0x3d, 0x3d, 0x3d, 0x36, 0x49, 0x49, 0x2d, 0x60, 0x82, 0x6f, 0x2d, 0x60, 0x7d, 0x6e, 0x70, 0x72,
        0x2d, 0x5a, 0x72, 0x80, 0x80, 0x6e, 0x74, 0x72, 0x2d, 0x4b, 0x4b, 0x1a, 0x53, 0x5f, 0x5c, 0x5a,
        0x47, 0x2d, 0x61, 0x75, 0x72, 0x2d, 0x5f, 0x72, 0x6f, 0x72, 0x79, 0x2d, 0x50, 0x7c, 0x7b, 0x73,
        0x72, 0x71, 0x72, 0x7f, 0x6e, 0x81, 0x76, 0x7c, 0x7b, 0x1a, 0x61, 0x5c, 0x47, 0x2d, 0x61, 0x75,
        0x72, 0x2d, 0x50, 0x86, 0x6f, 0x7c, 0x7f, 0x74, 0x1a, 0x1a, 0x4b, 0x2d, 0x4b, 0x2d, 0x51, 0x8e,
        0x7f, 0x73, 0x81, 0x72, 0x2d, 0x76, 0x70, 0x75, 0x2d, 0x71, 0x76, 0x70, 0x75, 0x2d, 0x72, 0x76,
        0x74, 0x72, 0x7b, 0x81, 0x79, 0x76, 0x70, 0x75, 0x2d, 0x6f, 0x76, 0x81, 0x81, 0x72, 0x7b, 0x39,
        0x2d, 0x83, 0x1a, 0x4b, 0x2d, 0x4b, 0x2d, 0x4e, 0x79, 0x76, 0x72, 0x7b, 0x6e, 0x71, 0x7c, 0x7b,
        0x76, 0x6e, 0x2d, 0x35, 0x30, 0x40, 0x41, 0x46, 0x36, 0x2d, 0x87, 0x82, 0x2d, 0x83, 0x72, 0x7f,
        0x80, 0x70, 0x75, 0x84, 0x76, 0x7b, 0x71, 0x72, 0x7b, 0x4c, 0x1a, 0x4b, 0x2d, 0x5d, 0x7f, 0x7c,
        0x6f, 0x76, 0x72, 0x7f, 0x34, 0x80, 0x2d, 0x71, 0x7c, 0x70, 0x75, 0x2d, 0x35, 0x71, 0x6e, 0x80,
        0x2d, 0x6f, 0x76, 0x81, 0x81, 0x72, 0x7b, 0x2d, 0x7a, 0x72, 0x76, 0x7b, 0x72, 0x2d, 0x76, 0x70,
        0x75, 0x36, 0x3b, 0x1a, 0x60, 0x70, 0x75, 0x7c, 0x7b, 0x2d, 0x74, 0x72, 0x80, 0x70, 0x75, 0x72,
        0x75, 0x72, 0x7b, 0x3b, 0x1a, 0x4b, 0x2d, 0x4b, 0x2d, 0x4e, 0x7b, 0x80, 0x7c, 0x7b, 0x80, 0x81,
        0x72, 0x7b, 0x2d, 0x84, 0x72, 0x7f, 0x71, 0x72, 0x2d, 0x76, 0x70, 0x75, 0x2d, 0x7a, 0x6e, 0x79,
        0x2d, 0x7a, 0x72, 0x76, 0x7b, 0x72, 0x2d, 0x80, 0x7d, 0x72, 0x70, 0x76, 0x6e, 0x1a, 0x4b, 0x2d,
        0x4b, 0x2d, 0x7a, 0x76, 0x80, 0x80, 0x76, 0x7c, 0x7b, 0x2d, 0x6e, 0x82, 0x80, 0x7d, 0x7f, 0x7c,
        0x6f, 0x76, 0x72, 0x7f, 0x72, 0x7b, 0x3b, 0x3b, 0x3b, 0x1a, 0x4b, 0x2d, 0x60, 0x7d, 0x76, 0x72,
        0x79, 0x83, 0x72, 0x7f, 0x71, 0x72, 0x7f, 0x6f, 0x72, 0x7f, 0x3b, 0x1a, 0x5b, 0x6e, 0x2d, 0x82,
        0x7b, 0x71, 0x4c, 0x1a, 0x4b, 0x2d, 0x4b, 0x2d, 0x56, 0x70, 0x75, 0x2d, 0x75, 0x6e, 0x6f, 0x72,
    };

    afl::string::NullTranslator tx;
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
    afl::io::ConstMemoryStream ms(DATA);
    game::v3::InboxFile testee(ms, cs, tx);

    TS_ASSERT_EQUALS(testee.getNumMessages(), 3U);
    TS_ASSERT_EQUALS(testee.loadMessage(0),
                     "(-h000)<<< Sub Space Message >>>\n"
                     "FROM: Host\n"
                     "TO: Everybody\n"
                     "n\xC3\xA4""chster host: mo 24.1.2000");
    TS_ASSERT_EQUALS(testee.loadMessage(1),
                     "(-r6000)<< Sub Space Message >>\n"
                     "FROM: The Cyborg\n"
                     "TO: The Cyborg\n"
                     "  <<< Universal Message >>>\n"
                     "\n"
                     "--- Forwarded Message ---\n"
                     "(-p0943)<<< Sub Space Message >>>\n"
                     "\n"
                     "Dresden-L\xC3\xB6""btau\n"
                     "Has been taken over by the\n"
                     "Empire ship\n"
                     "SMALL DEEP SPACE FREI\n"
                     "--- End Forwarded Message ---\n"
                     "\n"
                     "Arrrg! Wer hat den VCR verbrochen!\n"
                     "der zeigt das falsch rum an!");
    TS_ASSERT_EQUALS(testee.loadMessage(2),
                     "(-r9000)<< Sub Space Message >>\n"
                     "FROM: The Robotic Imperium\n"
                     "TO: The Cyborg\n"
                     "\n"
                     "Allied Planets\n"
                     "Passcode : 1520\n"
                     "2 9 519 36 44 47 48 87 115 145\n"
                     "154 170 171 174 193 199 214 218 728 248\n"
                     "257 261 269 277 278 280 284 286 292 305\n"
                     "315 319 342 344 352 359 386 393 394 455\n"
                     "461 470 471");
}

