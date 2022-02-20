/**
  *  \file server/file/serverapplication.hpp
  *  \brief Class server::file::ServerApplication
  */
#ifndef C2NG_SERVER_FILE_FILEAPPLICATION_HPP
#define C2NG_SERVER_FILE_FILEAPPLICATION_HPP

#include "afl/async/interrupt.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/application.hpp"

namespace server { namespace file {

    /** c2file server application.
        c2file-server's main function consists of an instantiation of this object. */
    class ServerApplication : public server::Application {
     public:
        /** Constructor.
            \param env  Environment
            \param fs   File system
            \param net  Network stack
            \param intr Operating system interrupts */
        ServerApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net, afl::async::Interrupt& intr);

        ~ServerApplication();

        virtual bool handleCommandLineOption(const String_t& option, afl::sys::CommandLineParser& parser);
        virtual void serverMain();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual String_t getApplicationName() const;
        virtual String_t getCommandLineOptionHelp() const;

     private:
        afl::net::Name m_listenAddress;
        String_t m_instanceName;                     // ex arg_instance
        String_t m_rootDirectory;                    // ex arg_basedir
        afl::io::Stream::FileSize_t m_maxFileSize;   // ex arg_file_size_limit
        afl::async::Interrupt& m_interrupt;
        bool m_gcEnabled;
    };

} }

#endif
