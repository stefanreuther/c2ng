/**
  *  \file server/file/clientapplication.hpp
  *  \brief Class server::file::ClientApplication
  */
#ifndef C2NG_SERVER_FILE_CLIENTAPPLICATION_HPP
#define C2NG_SERVER_FILE_CLIENTAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/sys/environment.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/net/tunnel/tunnelablenetworkstack.hpp"

namespace server { namespace file {

    class DirectoryHandler;

    /** c2fileclient application. */
    class ClientApplication : public util::Application {
     public:
        /** Constructor.
            \param env Environment
            \param fs File system
            \param net Network stack */
        ClientApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        virtual void appMain();

        afl::net::NetworkStack& networkStack();

     private:
        void doCopy(afl::sys::CommandLineParser& cmdl);
        void doSync(afl::sys::CommandLineParser& cmdl);
        void doList(afl::sys::CommandLineParser& cmdl);
        void doList(DirectoryHandler& in, String_t name, bool recursive, bool longFormat, bool withHeader);
        void doClear(afl::sys::CommandLineParser& cmdl);
        void doServe(afl::sys::CommandLineParser& cmdl);
        void help();

        afl::net::NetworkStack& m_serverNetworkStack;
        afl::net::tunnel::TunnelableNetworkStack m_networkStack;
    };

} }


inline
server::file::ClientApplication::ClientApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
    : Application(env, fs),
      m_serverNetworkStack(net),
      m_networkStack(net)
{ }

inline afl::net::NetworkStack&
server::file::ClientApplication::networkStack()
{
    return m_networkStack;
}


#endif
