/**
  *  \file game/v3/check/application.hpp
  *  \brief Class game::v3::check::Application
  */
#ifndef C2NG_GAME_V3_CHECK_APPLICATION_HPP
#define C2NG_GAME_V3_CHECK_APPLICATION_HPP

#include "util/application.hpp"

namespace game { namespace v3 { namespace check {

    class Application : public util::Application {
     public:
        Application(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        void help();
    };

} } }

#endif
