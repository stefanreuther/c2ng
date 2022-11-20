/**
  *  \file testapps/msgparse.cpp
  */

#include <cstring>
#include <iostream>
#include <stdexcept>
#include "game/parser/messageparser.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/textfile.hpp"
#include "game/parser/datainterface.hpp"
#include "game/parser/messageinformation.hpp"
#include "afl/string/parse.hpp"

namespace {
    class Logger : public afl::sys::LogListener {
     public:
        virtual void handleMessage(const Message& msg)
            { std::cout << msg.m_message << "\n"; }
    };

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

    class DataInterface : public game::parser::DataInterface {
     public:
        DataInterface(int playerNumber)
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

        int m_playerNumber;
    };

    const char*const LOG_NAME = "msgparse";
    Logger logger;
    afl::string::NullTranslator tx;
    game::parser::MessageParser parser;
    int turnNumber = 1;
    int playerNumber = 1;

    bool strStartsWith(const String_t& big, const char* small)
    {
        // FIXME: should be in library
        size_t n = std::strlen(small);
        return big.size() >= n
            && big.compare(0, n, small, n) == 0;
    }

    String_t toString(game::parser::MessageInformation::Type type)
    {
        switch (type) {
         case game::parser::MessageInformation::Ship:          return "Ship";
         case game::parser::MessageInformation::Planet:        return "Planet";
         case game::parser::MessageInformation::Starbase:      return "Starbase";
         case game::parser::MessageInformation::Minefield:     return "Minefield";
         case game::parser::MessageInformation::IonStorm:      return "IonStorm";
         case game::parser::MessageInformation::Ufo:           return "Ufo";
         case game::parser::MessageInformation::Wormhole:      return "Wormhole";
         case game::parser::MessageInformation::Explosion:     return "Explosion";
         case game::parser::MessageInformation::Configuration: return "Configuration";
         case game::parser::MessageInformation::PlayerScore:   return "PlayerScore";
         case game::parser::MessageInformation::Alliance:      return "Alliance";
         case game::parser::MessageInformation::NoObject:      return "NoObject";
         case game::parser::MessageInformation::MarkerDrawing: return "MarkerDrawing";
         case game::parser::MessageInformation::CircleDrawing: return "CircleDrawing";
         case game::parser::MessageInformation::LineDrawing:   return "LineDrawing";
         case game::parser::MessageInformation::RectangleDrawing: return "RectangleDrawing";
         case game::parser::MessageInformation::ExtraShip:     return "ExtraShip";
         case game::parser::MessageInformation::ExtraPlanet:   return "ExtraPlanet";
         case game::parser::MessageInformation::ExtraMinefield: return "ExtraMinefield";
        }
        return "?";
    }

    void loadTemplates(const char* fn)
    {
        afl::base::Ref<afl::io::Stream> file = afl::io::FileSystem::getInstance().openFile(fn, afl::io::FileSystem::OpenRead);
        parser.load(*file, tx, logger);
    }

    void parseSingleMessage(String_t message, int turnNumber)
    {
        if (message.empty()) {
            return;
        }

        DataInterface iface(playerNumber);
        afl::container::PtrVector<game::parser::MessageInformation> result;
        parser.parseMessage(message, iface, turnNumber, result, tx, logger);

        std::cout << "--- Parsed Message:\n"
                  << message;
        for (size_t i = 0; i < result.size(); ++i) {
            const game::parser::MessageInformation& info = *result[i];
            std::cout << "| " << toString(info.getObjectType()) << " #" << info.getObjectId() << ", turn " << info.getTurnNumber() << "\n";
            for (game::parser::MessageInformation::Iterator_t it = info.begin(); it != info.end(); ++it) {
                if (const game::parser::MessageStringValue_t* sv = dynamic_cast<game::parser::MessageStringValue_t*>(*it)) {
                    std::cout << "|    " << getNameFromIndex(sv->getIndex(), tx) << ": " << sv->getValue() << "\n";
                } else if (const game::parser::MessageIntegerValue_t* iv = dynamic_cast<game::parser::MessageIntegerValue_t*>(*it)) {
                    std::cout << "|    " << getNameFromIndex(iv->getIndex(), tx) << ": " << iv->getValue() << "\n";
                } else if (const game::parser::MessageConfigurationValue_t* cv = dynamic_cast<game::parser::MessageConfigurationValue_t*>(*it)) {
                    std::cout << "|    Config: " << cv->getIndex() << " = " << cv->getValue() << "\n";
                } else if (const game::parser::MessageScoreValue_t* scv = dynamic_cast<game::parser::MessageScoreValue_t*>(*it)) {
                    std::cout << "|    Player " << scv->getIndex() << " score: " << scv->getValue() << "\n";
                } else if (const game::parser::MessageAllianceValue_t* av = dynamic_cast<game::parser::MessageAllianceValue_t*>(*it)) {
                    std::cout << "|    Alliance offer " << av->getIndex() << "\n";
                } else {
                    std::cout << "|    (unknown)\n";
                }
            }
        }
    }

    void parseMessages(const char* fn)
    {
        afl::base::Ref<afl::io::Stream> file = afl::io::FileSystem::getInstance().openFile(fn, afl::io::FileSystem::OpenRead);
        afl::io::TextFile tf(*file);

        String_t line;
        String_t message;
        int thisTurnNumber = turnNumber;
        while (tf.readLine(line)) {
            if (strStartsWith(line, "--- Message")) {
                parseSingleMessage(message, thisTurnNumber);
                message.clear();
                thisTurnNumber = turnNumber;
            } else if (message.empty() && strStartsWith(line, "TURN:")) {
                afl::string::strToInteger(line.substr(5), thisTurnNumber);
            } else {
                message += line;
                message += "\n";
            }
        }
    }
}


int main(int, char** argv)
{
    try {
        while (const char* p = *++argv) {
            if (std::strcmp(p, "-help") == 0) {
            } else if (std::strncmp(p, "-load=", 6) == 0) {
                loadTemplates(p+6);
            } else if (*p == '-') {
                std::cerr << "Unknown option: " << p << std::endl;
                return 1;
            } else {
                parseMessages(p);
            }
        }
    }
    catch (std::exception& e) {
        logger.write(afl::sys::LogListener::Error, LOG_NAME, "Exception", e);
    }
}
