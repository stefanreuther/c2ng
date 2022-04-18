/**
  *  \file game/proxy/exportadaptor.hpp
  *  \brief Interface game::proxy::ExportAdaptor
  */
#ifndef C2NG_GAME_PROXY_EXPORTADAPTOR_HPP
#define C2NG_GAME_PROXY_EXPORTADAPTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "interpreter/context.hpp"
#include "interpreter/exporter/configuration.hpp"

namespace game { namespace proxy {

    /** Adaptor to for exporting data.
        Provides access to a Context, Configuration, and related objects. */
    class ExportAdaptor : public afl::base::Deletable {
     public:
        /** Initialize configuration.
            Updates the given configuration with default/least-recently-used data.
            Can be implemented empty.
            @param [out] config Configuration */
        virtual void initConfiguration(interpreter::exporter::Configuration& config) = 0;

        /** Save configuration.
            Called after every configuration change, to store the configuration for a potential future use.
            Can be implemented empty.
            @param config Configuration */
        virtual void saveConfiguration(const interpreter::exporter::Configuration& config) = 0;

        /** Create Context object providing data to export.
            Must create a fresh object that is used to provide fields and data.
            Each call must create a new, independant instance (e.g. by cloning a template).

            This function is allowed to return null;
            the responsibility to deal with that is with the caller (=proxy).

            @return Newly-allocated context, caller assumes responsibility. Can be null */
        virtual interpreter::Context* createContext() = 0;

        /** Access FileSystem instance.
            @return FileSystem */
        virtual afl::io::FileSystem& fileSystem() = 0;

        /** Access Translator instance (for error reports).
            @return Translator */
        virtual afl::string::Translator& translator() = 0;
    };

} }

#endif
