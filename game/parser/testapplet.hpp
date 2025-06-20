/**
  *  \file game/parser/testapplet.hpp
  *  \brief Class game::parser::TestApplet
  */
#ifndef C2NG_GAME_PARSER_TESTAPPLET_HPP
#define C2NG_GAME_PARSER_TESTAPPLET_HPP

#include "util/applet.hpp"
#include "game/parser/messageparser.hpp"

namespace game { namespace parser {

    /** Message parser test applet. */
    class TestApplet : public util::Applet {
     public:
        virtual int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        MessageParser m_parser;

        void loadTemplates(util::Application& app, const String_t& fn);
        void parseMessages(util::Application& app, const String_t& fn);
        void parseSingleMessage(util::Application& app, String_t message, int turnNumber);
    };

} }

#endif
