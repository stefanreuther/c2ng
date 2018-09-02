/**
  *  \file game/v3/maketurnapplication.hpp
  *  \brief Class game::v3::MaketurnApplication
  */
#ifndef C2NG_GAME_V3_MAKETURNAPPLICATION_HPP
#define C2NG_GAME_V3_MAKETURNAPPLICATION_HPP

#include "util/application.hpp"

namespace game { namespace v3 {

    class MaketurnApplication : public util::Application {
     public:
        MaketurnApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        void help();
    };

} }

#endif
