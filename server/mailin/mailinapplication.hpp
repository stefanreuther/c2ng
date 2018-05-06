/**
  *  \file server/mailin/mailinapplication.hpp
  *  \brief Class server::mailin::MailInApplication
  */
#ifndef C2NG_SERVER_MAILIN_MAILINAPPLICATION_HPP
#define C2NG_SERVER_MAILIN_MAILINAPPLICATION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/application.hpp"

namespace server { namespace mailin {

    /** c2mailin application. */
    class MailInApplication : public server::Application {
     public:
        MailInApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        void readMail(afl::io::Stream& buffer);
        bool saveRejectedMail(afl::base::ConstBytes_t buffer);

        bool m_dump;
        afl::net::Name m_hostAddress;
        afl::net::Name m_mailAddress;
        String_t m_rejectDirectory;
    };

} }

#endif
