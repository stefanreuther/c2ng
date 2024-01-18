/**
  *  \file test/game/msg/formattest.cpp
  *  \brief Test for game::msg::Format
  */

#include "game/msg/format.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/playerlist.hpp"
#include "game/test/files.hpp"
#include "game/v3/utils.hpp"

namespace {
    void prepareRaceNames(game::PlayerList& playerList)
    {
        afl::base::Ref<afl::io::InternalDirectory> dir = afl::io::InternalDirectory::create("spec");
        dir->addStream("race.nm", *new afl::io::ConstMemoryStream(game::test::getDefaultRaceNames()));
        afl::charset::CodepageCharset cs(afl::charset::g_codepage437);
        game::v3::loadRaceNames(playerList, *dir, cs);
    }
}

/*
 *  formatMessage()
 *
 *  Messages taken from actual games.
 */

// Test cases
// - PHost German, with coordinates
AFL_TEST("game.msg.Format:formatMessage:coordinates", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-m0090)<<< Minenbericht >>>\n"
                                                     "\n"
                                                     "von unserer Flotte:\n"
                                                     "\n"
                                                     "Wir haben Tholian-Fangminen entdeckt!\n"
                                                     "Dieses Minenfeld (ID #90) um\n"
                                                     "(2185, 1610) besteht aus\n"
                                                     "781 Minen und hat einen\n"
                                                     "Durchmesser von 54 Lichtjahren.\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference(game::map::Point(2185, 1610)));
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::Minefield, 90));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - Unspecial, but from host
AFL_TEST("game.msg.Format:formatMessage:from-host", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(og0000)< Message from your Host >\n"
                                                     " Free fighters at starbases\n"
                                                     "  Privateer    0\n"
                                                     "  Empire       10\n"
                                                     "web mine decay   5 %\n"
                                                     "mine decay       5 %\n"
                                                     "max mine radius  150\n"
                                                     "isotope TUDR     5\n"
                                                     "structure decay  1    \n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(0));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t(0));
}

// - Universal message from GH
AFL_TEST("game.msg.Format:formatMessage:from-gh", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-h000)<<< Sub Space Message >>>\n"
                                                     "FROM: Host\n"
                                                     "TO: Everybody\n"
                                                     "\n"
                                                     "next host: when you're done\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(0));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t::allUpTo(11));
}

// - Message to many
AFL_TEST("game.msg.Format:formatMessage:to-multiple", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(or2000)<< Sub Space Message >>\n"
                                                     "FROM: The Lizard Alliance\n"
                                                     "TO: The Rebel Confederation\n"
                                                     "CC: 6 8 9\n"
                                                     "\n"
                                                     "--- Forwarded Message ---\n"
                                                     "(-f0263)<<< Fleet Message >>>\n"
                                                     "Aeolos suXa domeol\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(2));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t() + 2 + 6 + 8 + 9 + 10);
}

// - Same thing, but mismatching race names
AFL_TEST("game.msg.Format:formatMessage:to-multiple:mismatching", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(or2000)<< Sub Space Message >>\n"
                                                     "FROM: Freihaendler von Bosycs Stern\n"
                                                     "TO: Die Chemnitzer Kolonien\n"
                                                     "CC: 6 8 9\n"
                                                     "\n"
                                                     "--- Forwarded Message ---\n"
                                                     "(-f0263)<<< Fleet Message >>>\n"
                                                     "Aeolos suXa domeol\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(2));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t() + 2 + 6 + 8 + 9);
}

// - Universal message, with mismatching names
AFL_TEST("game.msg.Format:formatMessage:universal", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(or9000)<< Sub Space Message >>\n"
                                                     "FROM: The Robotic Imperium\n"
                                                     "TO: Die Chemnitzer Kolonien\n"
                                                     "  <<< Universal Message >>>\n"
                                                     "\n"
                                                     "--- Forwarded Message ---\n"
                                                     "(-lame!)<<< Sub Space Message >>>\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(9));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t::allUpTo(11) - 0);
}

// - Totally not special
AFL_TEST("game.msg.Format:formatMessage:unspecial", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
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
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - Anonymous
AFL_TEST("game.msg.Format:formatMessage:anonymous", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-r0000)<<< Sub Space Message >>>\n"
                                                     "FROM: ? <0>\n"
                                                     "TO  : The Crystal Confederation\n"
                                                     "\n"
                                                     "i think fed and cyborg will win the\n"
                                                     "game.\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t::allUpTo(11) - 0);
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t::allUpTo(11) - 0);
}

// - Multiple coordinates
AFL_TEST("game.msg.Format:formatMessage:multiple-coordinates", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-m0005)<< Long Range Sensors >>\n"
                                                     "Mine fields exploding!\n"
                                                     "Between ( 2385, 1796)\n"
                                                     "and ( 2392, 1823)\n"
                                                     " 1538 explosions detected!\n", playerList, tx);
    a.checkEqual("firstLink",  msg.firstLink, game::Reference(game::map::Point(2385, 1796)));
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::Minefield, 5));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - c2nu
AFL_TEST("game.msg.Format:formatMessage:c2nu-player-message", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-r1000)<<< Subspace Message >>>\n"
                                                     "\n"
                                                     "From: The Solar Federation (madinson)\n"
                                                     "\n"
                                                     "Thanks to all, see you in another universe!\n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference());
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t(1));
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t(1));
}

// - planet
AFL_TEST("game.msg.Format:formatMessage:planet-message", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-y0181)<<< Meteorbericht >>>\n"
                                                     "\n"
                                                     "Von: Planet Garon IV\n"
                                                     "ID:  #181\n"
                                                     "\n"
                                                     "Meteoritenschauer! Es entstand\n"
                                                     "kein nennenswerter Schaden. Die\n"
                                                     "Meteoriten bestanden aus\n"
                                                     "...\n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::Planet, 181));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - ship
AFL_TEST("game.msg.Format:formatMessage:ship-message", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-s0010)<< Transporter Log >>\n"
                                                     "\n"
                                                     "From: Incompetent Freedom\n"
                                                     "Trying to beam cargo up from\n"
                                                     "another race's planet #365\n"
                                                     "Qvarne\n"
                                                     " 0 KT of neutronium\n"
                                                     "beamed up from the surface\n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::Ship, 10));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - ion storm
AFL_TEST("game.msg.Format:formatMessage:ion-storm-message", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-i0036)<<< ION Advisory >>>\n"
                                                     "Ion Disturbance\n"
                                                     "ID Number:  36\n"
                                                     "Centered At: (  1762, 2067)\n"
                                                     "East of \n"
                                                     "Planet ID Number  60\n"
                                                     " 86 LY from planet\n"
                                                     "Voltage : 123\n"
                                                     "Heading : 77\n"
                                                     "Speed   :  Warp 6\n"
                                                     "Radius  : 152\n"
                                                     "Class :  Level 3\n"
                                                     "  Strong\n"
                                                     "System is growing\n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference(game::map::Point(1762, 2067)));
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::IonStorm, 36));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}

// - space dock
AFL_TEST("game.msg.Format:formatMessage:space-dock-message", a)
{
    afl::string::NullTranslator tx;
    game::PlayerList playerList;
    prepareRaceNames(playerList);
    game::msg::Format msg = game::msg::formatMessage("(-d0279)<<< Space Dock Message >>>\n"
                                                     "\n"
                                                     "A new VICTORIOUS CLASS BATTLESHIP\n"
                                                     "Has been constructed\n"
                                                     "at Pedmont\n"
                                                     "space dock.\n", playerList, tx);

    a.checkEqual("firstLink",  msg.firstLink, game::Reference());
    a.checkEqual("headerLink", msg.headerLink, game::Reference(game::Reference::Starbase, 279));
    a.checkEqual("reply",      msg.reply, game::PlayerSet_t());
    a.checkEqual("replyAll",   msg.replyAll, game::PlayerSet_t());
}


/** Test quoteMessageForReply().
    Messages taken from actual games.  */
AFL_TEST("game.msg.Format:quoteMessageForReply", a)
{
    // Standard case
    a.checkEqual("01", game::msg::quoteMessageForReply("(-h000)<<< Sub Space Message >>>\n"
                                                       "FROM: Host\n"
                                                       "TO: Everybody\n"
                                                       "\n"
                                                       "next host: when you're done\n"),
                 "> next host: when you're done\n");

    // c2nu
    a.checkEqual("11", game::msg::quoteMessageForReply("(-r1000)<<< Subspace Message >>>\n"
                                                       "\n"
                                                       "From: The Solar Federation (madinson)\n"
                                                       "\n"
                                                       "Thanks to all, see you in another universe!\n"),
                 "> Thanks to all, see you in another universe!\n");

    // Without () header
    a.checkEqual("21", game::msg::quoteMessageForReply("<<< HSScore 2.01 >>>\n"
                                                       "\n"
                                                       "You are using the HSScore scoring\n"
                                                       "system. For a description of the\n"),
                 "> You are using the HSScore scoring\n"
                 "> system. For a description of the\n");

    // Without () header, with From header
    a.checkEqual("31", game::msg::quoteMessageForReply("<<< The Machines of Yore >>>\n"
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
    a.checkEqual("41", game::msg::quoteMessageForReply("(-r7000)<<< Subraumnachricht >>>\n"
                                                       "Von : The Tholian Holdfast <7>\n"
                                                       "An  : The Animal Farm\n"
                                                       "> Ganz nebenbei, ich kann jedoch\n"),
                 ">> Ganz nebenbei, ich kann jedoch\n");
}
