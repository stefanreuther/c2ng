/**
  *  \file test/game/proxy/configurationeditoradaptortest.cpp
  *  \brief Test for game::proxy::ConfigurationEditorAdaptor
  */

#include "game/proxy/configurationeditoradaptor.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

/** Interface test. */
AFL_TEST_NOARG("game.proxy.ConfigurationEditorAdaptor")
{
    class Tester : public game::proxy::ConfigurationEditorAdaptor {
     public:
        virtual game::config::Configuration& config()
            { throw std::runtime_error("ref"); }
        virtual game::config::ConfigurationEditor& editor()
            { throw std::runtime_error("ref"); }
        virtual afl::string::Translator& translator()
            { throw std::runtime_error("ref"); }
        virtual void notifyListeners()
            { }
    };
    Tester t;
}
