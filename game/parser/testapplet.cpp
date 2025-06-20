/**
  *  \file game/parser/testapplet.cpp
  *  \brief Class game::parser::TestApplet
  */

#include "game/parser/testapplet.hpp"

#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageinformation.hpp"
#include "util/string.hpp"

using afl::base::Ref;
using afl::container::PtrVector;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextWriter;
using afl::string::Format;
using afl::string::Translator;
using game::parser::MessageInformation;

namespace {
    const int TURN_NUMBER = 1;
    const int PLAYER_NUMBER = 1;

    const char*const NAMES[11][3] = {
        { "The Solar Federation", "The Feds", "Fed" },
        { "The Lizard Alliance", "The Lizards", "Lizard" },
        { "The Empire of the Birds", "The Bird Men", "Bird Man" },
        { "The Fascist Empire", "The Fascists", "Fascist" },
        { "The Privateer Bands", "The Privateers", "Privateer" },
        { "The Cyborg", "The Cyborg", "Cyborg" },
        { "The Crystal Confederation", "The Crystal People", "Crystalline" },
        { "The Evil Empire", "The Evil Empire", "Empire" },
        { "The Robotic Imperium", "The Robots", "Robotic" },
        { "The Rebel Confederation", "The Rebels", "Rebel" },
        { "The Missing Colonies of Man", "The Colonies", "Colonial" },
    };

    class TestDataInterface : public game::parser::DataInterface {
     public:
        TestDataInterface(int playerNumber)
            : m_playerNumber(playerNumber)
            { }
        virtual int getPlayerNumber() const
            { return m_playerNumber; }

        virtual int parseName(Name which, const String_t& name) const
            {
                switch (which) {
                 case ShortRaceName:
                    return parsePlayerName(1, name);
                 case LongRaceName:
                    return parsePlayerName(0, name);
                 case AdjectiveRaceName:
                    return parsePlayerName(2, name);
                 case HullName:
                    return 0;
                }
                return 0;
            }

        virtual String_t expandRaceNames(String_t tpl) const
            {
                // Unsupported for now
                return tpl;
            }

     private:
        static int parsePlayerName(int slot, const String_t& name)
            {
                for (int i = 1; i <= 11; ++i) {
                    if (NAMES[i-1][slot] == name) {
                        return i;
                    }
                }
                return 0;
            }

        const int m_playerNumber;
    };

    String_t toString(MessageInformation::Type type)
    {
        switch (type) {
         case MessageInformation::Ship:             return "Ship";
         case MessageInformation::Planet:           return "Planet";
         case MessageInformation::Starbase:         return "Starbase";
         case MessageInformation::Minefield:        return "Minefield";
         case MessageInformation::IonStorm:         return "IonStorm";
         case MessageInformation::Ufo:              return "Ufo";
         case MessageInformation::Wormhole:         return "Wormhole";
         case MessageInformation::Explosion:        return "Explosion";
         case MessageInformation::Configuration:    return "Configuration";
         case MessageInformation::PlayerScore:      return "PlayerScore";
         case MessageInformation::Alliance:         return "Alliance";
         case MessageInformation::NoObject:         return "NoObject";
         case MessageInformation::MarkerDrawing:    return "MarkerDrawing";
         case MessageInformation::CircleDrawing:    return "CircleDrawing";
         case MessageInformation::LineDrawing:      return "LineDrawing";
         case MessageInformation::RectangleDrawing: return "RectangleDrawing";
         case MessageInformation::ExtraShip:        return "ExtraShip";
         case MessageInformation::ExtraPlanet:      return "ExtraPlanet";
         case MessageInformation::ExtraMinefield:   return "ExtraMinefield";
        }
        return "?";
    }
}

int
game::parser::TestApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    String_t it;
    while (cmdl.getNextElement(it)) {
        if (it == "-help") {
            app.standardOutput().writeLine("usage: msgparse [-load=msgparse.ini] file...");
            return 0;
        } else if (const char* p = util::strStartsWith(it, "-load=")) {
            loadTemplates(app, p);
        } else if (util::strStartsWith(it, "-")) {
            app.errorOutput().writeLine(Format("Unknown option: %s", it));
            return 1;
        } else {
            parseMessages(app, it);
        }
    }
    return 0;
}

void
game::parser::TestApplet::loadTemplates(util::Application& app, const String_t& fn)
{
    Ref<Stream> file = app.fileSystem().openFile(fn, FileSystem::OpenRead);
    m_parser.load(*file, app.translator(), app.log());
}

void
game::parser::TestApplet::parseMessages(util::Application& app, const String_t& fn)
{
    Ref<Stream> file = app.fileSystem().openFile(fn, FileSystem::OpenRead);
    afl::io::TextFile tf(*file);

    String_t line;
    String_t message;
    int thisTurnNumber = TURN_NUMBER;
    while (tf.readLine(line)) {
        if (util::strStartsWith(line, "--- Message")) {
            parseSingleMessage(app, message, thisTurnNumber);
            message.clear();
            thisTurnNumber = TURN_NUMBER;
        } else if (message.empty() && util::strStartsWith(line, "TURN:")) {
            afl::string::strToInteger(line.substr(5), thisTurnNumber);
        } else {
            message += line;
            message += "\n";
        }
    }
    parseSingleMessage(app, message, thisTurnNumber);
}

void
game::parser::TestApplet::parseSingleMessage(util::Application& app, String_t message, int turnNumber)
{
    if (message.empty()) {
        return;
    }

    TestDataInterface iface(PLAYER_NUMBER);
    PtrVector<MessageInformation> result;
    m_parser.parseMessage(message, iface, turnNumber, result, app.translator(), app.log());

    TextWriter& out = app.standardOutput();
    Translator& tx = app.translator();

    out.writeLine("--- Parsed Message:");
    out.writeText(message);
    for (size_t i = 0; i < result.size(); ++i) {
        const MessageInformation& info = *result[i];
        out.writeLine(Format("| %s #%d, turn %d", toString(info.getObjectType()), info.getObjectId(), info.getTurnNumber()));
        for (MessageInformation::Iterator_t it = info.begin(); it != info.end(); ++it) {
            if (const MessageStringValue_t* sv = dynamic_cast<MessageStringValue_t*>(*it)) {
                out.writeLine(Format("|    %s: %s", getNameFromIndex(sv->getIndex(), tx), sv->getValue()));
            } else if (const MessageIntegerValue_t* iv = dynamic_cast<MessageIntegerValue_t*>(*it)) {
                out.writeLine(Format("|    %s: %d", getNameFromIndex(iv->getIndex(), tx), iv->getValue()));
            } else if (const MessageConfigurationValue_t* cv = dynamic_cast<MessageConfigurationValue_t*>(*it)) {
                out.writeLine(Format("|    Config: %s = %s", cv->getIndex(), cv->getValue()));
            } else if (const MessageScoreValue_t* scv = dynamic_cast<MessageScoreValue_t*>(*it)) {
                out.writeLine(Format("|    Player %d score: %d", scv->getIndex(), scv->getValue()));
            } else if (const MessageAllianceValue_t* av = dynamic_cast<MessageAllianceValue_t*>(*it)) {
                out.writeLine(Format("|    Alliance offer %d", av->getIndex()));
            } else {
                out.writeLine("|    (unknown)");
            }
        }
    }
}
