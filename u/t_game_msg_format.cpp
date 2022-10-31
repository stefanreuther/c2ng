/**
  *  \file u/t_game_msg_format.cpp
  *  \brief Test for game::msg::Format
  */

#include "game/msg/format.hpp"

#include "t_game_msg.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/playerlist.hpp"
#include "game/test/files.hpp"
#include "game/v3/utils.hpp"

/** Test formatMessage().
    Messages taken from actual games.  */
void
TestGameMsgFormat::testFormatMessage()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("spec");
    dir->addStream("race.nm", *new afl::io::ConstMemoryStream(game::test::getDefaultRaceNames()));
    afl::charset::CodepageCharset cs(afl::charset::g_codepage437);

    // Player list
    game::PlayerList playerList;
    game::v3::loadRaceNames(playerList, *dir, cs);

    // Test cases
    // - PHost German, with coordinates
    {
        game::msg::Format msg = game::msg::formatMessage("(-m0090)<<< Minenbericht >>>\n"
                                                         "\n"
                                                         "von unserer Flotte:\n"
                                                         "\n"
                                                         "Wir haben Tholian-Fangminen entdeckt!\n"
                                                         "Dieses Minenfeld (ID #90) um\n"
                                                         "(2185, 1610) besteht aus\n"
                                                         "781 Minen und hat einen\n"
                                                         "Durchmesser von 54 Lichtjahren.\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference(game::map::Point(2185, 1610)));
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t());
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t());
    }

    // - Unspecial, but from host
    {
        game::msg::Format msg = game::msg::formatMessage("(og0000)< Message from your Host >\n"
                                                         " Free fighters at starbases\n"
                                                         "  Privateer    0\n"
                                                         "  Empire       10\n"
                                                         "web mine decay   5 %\n"
                                                         "mine decay       5 %\n"
                                                         "max mine radius  150\n"
                                                         "isotope TUDR     5\n"
                                                         "structure decay  1    \n", playerList, tx);

        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(0));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t(0));
    }

    // - Universal message from GH
    {
        game::msg::Format msg = game::msg::formatMessage("(-h000)<<< Sub Space Message >>>\n"
                                                         "FROM: Host\n"
                                                         "TO: Everybody\n"
                                                         "\n"
                                                         "next host: when you're done\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(0));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t::allUpTo(11));
    }

    // - Message to many
    {
        game::msg::Format msg = game::msg::formatMessage("(or2000)<< Sub Space Message >>\n"
                                                         "FROM: The Lizard Alliance\n"
                                                         "TO: The Rebel Confederation\n"
                                                         "CC: 6 8 9\n"
                                                         "\n"
                                                         "--- Forwarded Message ---\n"
                                                         "(-f0263)<<< Fleet Message >>>\n"
                                                         "Aeolos suXa domeol\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t() + 2 + 6 + 8 + 9 + 10);
    }

    // - Same thing, but mismatching race names
    {
        game::msg::Format msg = game::msg::formatMessage("(or2000)<< Sub Space Message >>\n"
                                                         "FROM: Freihaendler von Bosycs Stern\n"
                                                         "TO: Die Chemnitzer Kolonien\n"
                                                         "CC: 6 8 9\n"
                                                         "\n"
                                                         "--- Forwarded Message ---\n"
                                                         "(-f0263)<<< Fleet Message >>>\n"
                                                         "Aeolos suXa domeol\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t() + 2 + 6 + 8 + 9);
    }

    // - Universal message, with mismatching names
    {
        game::msg::Format msg = game::msg::formatMessage("(or9000)<< Sub Space Message >>\n"
                                                         "FROM: The Robotic Imperium\n"
                                                         "TO: Die Chemnitzer Kolonien\n"
                                                         "  <<< Universal Message >>>\n"
                                                         "\n"
                                                         "--- Forwarded Message ---\n"
                                                         "(-lame!)<<< Sub Space Message >>>\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(9));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t::allUpTo(11) - 0);
    }

    // - Totally not special
    {
        game::msg::Format msg = game::msg::formatMessage("Turn: 55          \n"
                                                         "Race                used\n"
                                                         "------------------------\n"
                                                         " 8 The Evil Empire 17394\n"
                                                         " 2 The Lizards     16594\n"
                                                         " 3 The Bird Men    16594\n"
                                                         " 4 The Fascists    16594\n"
                                                         " 5 The Privateers  16594\n"
                                                         " 6 The Cyborg      16594\n"
                                                         " 7 The Crystal Peo 16594\n"
                                                         " 1 The Feds        16594\n"
                                                         " 9 The Robots      16594\n"
                                                         "11 The Colonies    15210\n"
                                                         "10 The Rebels      13826\n"
                                                         "------------------------\n"
                                                         "ptscore v1.4\n"
                                                         "\n"
                                                         "ship slots : 19 used, 481 empty\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t());
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t());
    }

    // - Anonymous
    {
        game::msg::Format msg = game::msg::formatMessage("(-r0000)<<< Sub Space Message >>>\n"
                                                         "FROM: ? <0>\n"
                                                         "TO  : The Crystal Confederation\n"
                                                         "\n"
                                                         "i think fed and cyborg will win the\n"
                                                         "game.\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t::allUpTo(11) - 0);
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t::allUpTo(11) - 0);
    }

    // - Multiple coordinates
    {
        game::msg::Format msg = game::msg::formatMessage("(-m0005)<< Long Range Sensors >>\n"
                                                         "Mine fields exploding!\n"
                                                         "Between ( 2385, 1796)\n"
                                                         "and ( 2392, 1823)\n"
                                                         " 1538 explosions detected!\n", playerList, tx);
        TS_ASSERT_EQUALS(msg.firstLink, game::Reference(game::map::Point(2385, 1796)));
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t());
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t());
    }

    // - c2nu
    {
        game::msg::Format msg = game::msg::formatMessage("(-r1000)<<< Subspace Message >>>\n"
                                                         "\n"
                                                         "From: The Solar Federation (madinson)\n"
                                                         "\n"
                                                         "Thanks to all, see you in another universe!\n", playerList, tx);

        TS_ASSERT_EQUALS(msg.firstLink, game::Reference());
        TS_ASSERT_EQUALS(msg.reply, game::PlayerSet_t(1));
        TS_ASSERT_EQUALS(msg.replyAll, game::PlayerSet_t(1));
    }
}

/** Test quoteMessageForReply().
    Messages taken from actual games.  */
void
TestGameMsgFormat::testQuoteMessageForReply()
{
    // Standard case
    TS_ASSERT_EQUALS(game::msg::quoteMessageForReply("(-h000)<<< Sub Space Message >>>\n"
                                                     "FROM: Host\n"
                                                     "TO: Everybody\n"
                                                     "\n"
                                                     "next host: when you're done\n"),
                     "> next host: when you're done\n");

    // c2nu
    TS_ASSERT_EQUALS(game::msg::quoteMessageForReply("(-r1000)<<< Subspace Message >>>\n"
                                                     "\n"
                                                     "From: The Solar Federation (madinson)\n"
                                                     "\n"
                                                     "Thanks to all, see you in another universe!\n"),
                     "> Thanks to all, see you in another universe!\n");

    // Without () header
    TS_ASSERT_EQUALS(game::msg::quoteMessageForReply("<<< HSScore 2.01 >>>\n"
                                                     "\n"
                                                     "You are using the HSScore scoring\n"
                                                     "system. For a description of the\n"),
                     "> You are using the HSScore scoring\n"
                     "> system. For a description of the\n");

    // Without () header, with From header
    TS_ASSERT_EQUALS(game::msg::quoteMessageForReply("<<< The Machines of Yore >>>\n"
                                                     "From: METEOR CLASS BLOCKAD\n"
                                                     "Ship ID# 457\n"
                                                     "\n"
                                                     "We have been caught in a large gravity\n"
                                                     "well!\n"),
                     "> Ship ID# 457\n"
                     ">\n"
                     "> We have been caught in a large gravity\n"
                     "> well!\n");

    // German, quoted
    TS_ASSERT_EQUALS(game::msg::quoteMessageForReply("(-r7000)<<< Subraumnachricht >>>\n"
                                                     "Von : The Tholian Holdfast <7>\n"
                                                     "An  : The Animal Farm\n"
                                                     "> Ganz nebenbei, ich kann jedoch\n"),
                     ">> Ganz nebenbei, ich kann jedoch\n");
}

