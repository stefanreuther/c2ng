/**
  *  \file util/plugin/dialogapplication.hpp
  *  \brief Class util::plugin::DialogApplication
  */
#ifndef C2NG_UTIL_PLUGIN_DIALOGAPPLICATION_HPP
#define C2NG_UTIL_PLUGIN_DIALOGAPPLICATION_HPP

#include <vector>
#include "afl/io/filesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/dialog.hpp"
#include "afl/sys/environment.hpp"
#include "util/application.hpp"

namespace util { namespace plugin {

    class Installer;

    /** Main function of a dialog-based plugin installer application (c2pluginw). */
    class DialogApplication : public Application {
     public:
        /** Constructor.
            @param env    Environment
            @param fs     File System
            @param dialog Dialog instance */
        DialogApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::sys::Dialog& dialog);

        // Application:
        virtual void appMain();

     private:
        afl::sys::Dialog& m_dialog;

        bool checkPreconditions(Installer& inst);
        void doAdd(const std::vector<String_t>& items);
        void die(const String_t& text);

        String_t windowTitle();
    };

} }

#endif
