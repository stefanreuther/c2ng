/**
  *  \file u/t_game_proxy_exportadaptor.cpp
  *  \brief Test for game::proxy::ExportAdaptor
  */

#include "game/proxy/exportadaptor.hpp"

#include "t_game_proxy.hpp"

/** Interface test. */
void
TestGameProxyExportAdaptor::testInterface()
{
    class Tester : public game::proxy::ExportAdaptor {
     public:
        virtual void initConfiguration(interpreter::exporter::Configuration& /*config*/)
            { }
        virtual void saveConfiguration(const interpreter::exporter::Configuration& /*config*/)
            { }
        virtual interpreter::Context* createContext()
            { return 0; }
        virtual afl::io::FileSystem& fileSystem()
            { throw "no ref"; }
        virtual afl::string::Translator& translator()
            { throw "no ref"; }
    };
    Tester t;
}

