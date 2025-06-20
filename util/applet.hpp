/**
  *  \file util/applet.hpp
  */
#ifndef C2NG_UTIL_APPLET_HPP
#define C2NG_UTIL_APPLET_HPP

#include "afl/sys/environment.hpp"
#include "util/application.hpp"
#include "afl/container/ptrvector.hpp"

namespace util {

    class Applet {
     public:
        virtual ~Applet()
            { }

        virtual int run(Application& app, afl::sys::Environment::CommandLine_t& cmdl) = 0;

        class Runner;
    };

    class Applet::Runner : public Application {
     public:
        Runner(String_t untranslatedName, afl::sys::Environment& env, afl::io::FileSystem& fs);

        ~Runner();

        Runner& addNew(String_t name, String_t untranslatedInfo, Applet* p);

     protected:
        virtual void appMain();

        struct Info;
        afl::container::PtrVector<Info> m_applets;
        String_t m_untranslatedName;

        const Info* findApplet(const String_t& appletName) const;
        void showHelp();
    };

}

#endif
