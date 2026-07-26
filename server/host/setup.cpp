/**
  *  \file server/host/setup.cpp
  *  \brief Host directory setup
  */

#include "server/host/setup.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "game/config/integerarrayoption.hpp"
#include "game/config/integervalueparser.hpp"
#include "server/common/racenames.hpp"
#include "server/interface/filebaseclient.hpp"

using server::common::RaceNames;
using server::host::Game;
using server::host::Root;

namespace {
    class RaceSelection : public game::config::IntegerArrayOption<Game::NUM_PLAYERS> {
     public:
        RaceSelection()
            : IntegerArrayOption(game::config::IntegerValueParser::instance)
            { }

        int add(int pos, int value)
            {
                if (!has(pos, value)) {
                    set(pos, value);
                    ++pos;
                }
                return pos;
            }
        bool has(int pos, int value) const
            {
                for (int i = 1; i <= pos; ++i) {
                    if ((*this)(i) == value) {
                        return true;
                    }
                }
                return false;
            }
        void complete(int pos)
            {
                for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
                    pos = add(pos, i);
                }
            }
    };

    void parseRaceChoice(RaceSelection& out, const String_t& in)
    {
        size_t i = 0;
        int pos = 1;
        while (i < in.size()) {
            size_t n = in.find(',', i);
            String_t seg = afl::string::strTrim(n == String_t::npos
                                                ? in.substr(i)
                                                : in.substr(i, n-i));
            try {
                int32_t value = out.parser().parse(seg);
                if (value >= 1 && value <= Game::NUM_PLAYERS) {
                    pos = out.add(pos, value);
                }
            }
            catch (std::range_error& e)
            { }

            if (n == String_t::npos) {
                break;
            } else {
                i = n+1;
            }
        }
        out.complete(pos);
    }

    void performPreHostSetupForDualDuel(Root& root, Game& game)
    {
        // Determine races
        RaceSelection p1, p2;
        parseRaceChoice(p1, game.getSlot(1).raceChoice().get());
        parseRaceChoice(p2, game.getSlot(2).raceChoice().get());

        RaceSelection playerRace;
        int race1, race2;
        if (p1(1) != p2(1)) {
            race1 = p1(1);
            race2 = p2(1);
        } else if (p1(2) != p2(2)) {
            race1 = p1(2);
            race2 = p2(2);
        } else {
            race1 = p1(1);
            race2 = p1(2);
        }
        playerRace.set(1, race1);
        playerRace.set(2, race2);
        playerRace.set(3, race2);
        playerRace.set(4, race1);
        playerRace.complete(5);

        // Set PlayerRace
        game.settings().stringField("playerRace").set(playerRace.toString());

        // Set race names
        afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
        server::interface::FileBaseClient client(root.hostFile());
        RaceNames eastNames, westNames, raceNames;
        eastNames.load(afl::string::toBytes(client.getFile("defaults/race-east.nm")), cs);
        westNames.load(afl::string::toBytes(client.getFile("defaults/race-west.nm")), cs);
        raceNames.load(afl::string::toBytes(client.getFile("defaults/race-none.nm")), cs);
        raceNames.copy(1, westNames, race1);
        raceNames.copy(2, westNames, race2);
        raceNames.copy(3, eastNames, race2);
        raceNames.copy(4, eastNames, race1);

        afl::base::GrowableBytes_t out;
        raceNames.save(out, cs);
        client.putFile(game.getDirectory() + "/data/race.nm", afl::string::fromBytes(out));

        // Set initial alliances
        client.putFile(game.getDirectory() + "/data/auxcmds.txt",
                       "1: allies add 3\n"
                       "2: allies add 4\n"
                       "3: allies add 1\n"
                       "4: allies add 2\n"
                       "1: allies config 3 +p +v\n"
                       "2: allies config 4 +p +v\n"
                       "3: allies config 1 +p +v\n"
                       "4: allies config 2 +p +v\n");
    }
}

/*
 *  Main Entry Point
 */

void
server::host::performPreHostSetup(Root& root, Game& game)
{
    switch (game.kind().get()) {
     case Game::Kind_DualDuel:
        performPreHostSetupForDualDuel(root, game);
        break;
    }
}
