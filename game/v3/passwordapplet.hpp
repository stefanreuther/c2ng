/**
  *  \file game/v3/passwordapplet.hpp
  *  \brief Class game::v3::PasswordApplet
  */
#ifndef C2NG_GAME_V3_PASSWORDAPPLET_HPP
#define C2NG_GAME_V3_PASSWORDAPPLET_HPP

#include "util/applet.hpp"

namespace game { namespace v3 {

    /** Password cracker applet. */
    class PasswordApplet : public util::Applet {
     public:
        virtual int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl);

     private:
        void handleFile(afl::io::TextWriter& out, afl::io::Stream& in, const String_t& fileName, afl::string::Translator& tx);
    };

} }

#endif
