/**
  *  \file game/v3/unpackapplication.hpp
  *  \brief Class game::v3::UnpackApplication
  */
#ifndef C2NG_GAME_V3_UNPACKAPPLICATION_HPP
#define C2NG_GAME_V3_UNPACKAPPLICATION_HPP

#include "util/application.hpp"

namespace game { namespace v3 {

    class ResultFile;
    class TurnFile;

    class UnpackApplication : public util::Application {
     public:
        UnpackApplication(afl::sys::Environment& env, afl::io::FileSystem& fs);

        virtual void appMain();

     private:
        static bool validateTurn(int player, ResultFile& rst, TurnFile& trn);
        void help();
    };

} }

#endif
