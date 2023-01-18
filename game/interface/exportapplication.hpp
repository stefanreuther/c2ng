/**
  *  \file game/interface/exportapplication.hpp
  *  \brief Class game::interface::ExportApplication
  */
#ifndef C2NG_GAME_INTERFACE_EXPORTAPPLICATION_HPP
#define C2NG_GAME_INTERFACE_EXPORTAPPLICATION_HPP

#include "interpreter/context.hpp"
#include "interpreter/world.hpp"
#include "util/application.hpp"

namespace game { namespace interface {

    /** c2export main aapplication. */
    class ExportApplication : public util::Application {
     public:
        /** Constructor.
            @param env Environment
            @param fs  File System */
        ExportApplication(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }

        /** Main entry point. */
        void appMain();

     private:
        void help();
        interpreter::Context* findArray(const String_t& name, interpreter::World& world);
    };

} }

#endif
