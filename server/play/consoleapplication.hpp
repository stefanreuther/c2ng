/**
  *  \file server/play/consoleapplication.hpp
  */
#ifndef C2NG_SERVER_PLAY_CONSOLEAPPLICATION_HPP
#define C2NG_SERVER_PLAY_CONSOLEAPPLICATION_HPP

#include <map>
#include "afl/base/ptr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "game/root.hpp"
#include "util/application.hpp"

namespace server { namespace play {

    class ConsoleApplication : public util::Application {
     public:
        ConsoleApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        virtual void appMain();

     private:
        struct Parameters;

        void help();

        afl::net::NetworkStack& m_network;
        std::map<String_t, String_t> m_properties;
        afl::io::NullFileSystem m_nullFileSystem;

        afl::base::Ptr<game::Root> loadRoot(const String_t& gameDir, const Parameters& params, afl::sys::LogListener& log);
    };

} }

#endif
