/**
  *  \file u/t_game_proxy_configurationeditoradaptor.cpp
  *  \brief Test for game::proxy::ConfigurationEditorAdaptor
  */

#include <exception>
#include "game/proxy/configurationeditoradaptor.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyConfigurationEditorAdaptor::testInterface()
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

