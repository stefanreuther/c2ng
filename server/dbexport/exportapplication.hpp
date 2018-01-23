/**
  *  \file server/dbexport/exportapplication.hpp
  *  \brief Class server::dbexport::ExportApplication
  */
#ifndef C2NG_SERVER_DBEXPORT_EXPORTAPPLICATION_HPP
#define C2NG_SERVER_DBEXPORT_EXPORTAPPLICATION_HPP

#include "util/application.hpp"
#include "afl/base/deleter.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/name.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/sys/environment.hpp"
#include "server/configurationhandler.hpp"

namespace server { namespace dbexport {

    /** c2dbexport main application. */
    class ExportApplication : public util::Application,
                              private ConfigurationHandler
    {
     public:
        /** Constructor.
            \param env Environment
            \param fs File system
            \param net Network stack */
        ExportApplication(afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        /** Destructor. */
        ~ExportApplication();

        /** Application enty point. */
        virtual void appMain();

     private:
        /** Network stack. */
        afl::net::NetworkStack& m_networkStack;

        /** Database address. */
        afl::net::Name m_dbAddress; // ex database_host, database_port, database_connection

        // ConfigurationHandler:
        virtual bool handleConfiguration(const String_t& key, const String_t& value);

        /** Show help screen and exit. */
        void help();

        /** Create a network client.
            \param del Deleter to control lifetime of created objects
            \param name Configured address
            \return Network client */
        afl::net::CommandHandler& createClient(afl::base::Deleter& del, const afl::net::Name& name);
    };

} }

#endif
