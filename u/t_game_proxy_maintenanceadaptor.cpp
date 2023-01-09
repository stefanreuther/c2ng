/**
  *  \file u/t_game_proxy_maintenanceadaptor.cpp
  *  \brief Test for game::proxy::MaintenanceAdaptor
  */

#include "game/proxy/maintenanceadaptor.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyMaintenanceAdaptor::testInterface()
{
    class Tester : public game::proxy::MaintenanceAdaptor {
     public:
        virtual afl::io::Directory& targetDirectory()
            { throw "no ref"; }
        virtual afl::string::Translator& translator()
            { throw "no ref"; }
        virtual afl::charset::Charset& charset()
            { throw "no ref"; }
        virtual const game::PlayerList& playerList()
            { throw "no ref"; }
        virtual afl::io::FileSystem& fileSystem()
            { throw "no ref"; }
        virtual game::config::UserConfiguration& userConfiguration()
            { throw "no ref"; }
    };
    Tester t;
}

