/**
  *  \file server/play/consoleapplication.hpp
  */
#ifndef C2NG_SERVER_PLAY_CONSOLEAPPLICATION_HPP
#define C2NG_SERVER_PLAY_CONSOLEAPPLICATION_HPP

#include <map>
#include "util/application.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/io/nullfilesystem.hpp"

namespace server { namespace play {

    class ConsoleApplication : public util::Application {
     public:
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        virtual void appMain();

     private:
        void help();

        std::map<String_t, String_t> m_properties;
        afl::io::NullFileSystem m_nullFileSystem;
    };

} }

#endif
