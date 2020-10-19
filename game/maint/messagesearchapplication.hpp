/**
  *  \file game/maint/messagesearchapplication.hpp
  *  \brief Class game::maint::MessageSearchApplication
  */
#ifndef C2NG_GAME_MAINT_MESSAGESEARCHAPPLICATION_HPP
#define C2NG_GAME_MAINT_MESSAGESEARCHAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/io/stream.hpp"

namespace game { namespace maint {

    class MessageSearchApplication : public util::Application {
     public:
        class Job;

        MessageSearchApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        void appMain();

     private:
        void searchZip(afl::io::Stream& file, String_t fname, const Job& job);
        void searchStream(afl::io::Stream& file, const String_t& fname, const Job& job);
        void searchFile(const String_t& fname, const Job& job);
        void help();
    };

} }

#endif
