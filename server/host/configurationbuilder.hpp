/**
  *  \file server/host/configurationbuilder.hpp
  *  \brief Class server::host::ConfigurationBuilder
  */
#ifndef C2NG_SERVER_HOST_CONFIGURATIONBUILDER_HPP
#define C2NG_SERVER_HOST_CONFIGURATIONBUILDER_HPP

#include "afl/base/memory.hpp"
#include "afl/string/string.hpp"

namespace server { namespace host {

    /** Configuration (c2game.ini) builder.
        This is a very simple class to format a set of key/value pairs into a c2game.ini file.
        The file is later sourced into a unix shell, hence we have to follow POSIX rules for variable names and content.

        This is just a formatter with no built-in logic for things like values overwriting previous values,
        read-back of previously set values, etc.
        It produces a byte buffer that can be written to a file. */
    class ConfigurationBuilder {
     public:
        /** Constructor.
            Makes an empty buffer. */
        ConfigurationBuilder();

        /** Destructor. */
        ~ConfigurationBuilder();

        /** Add a value.
            \param key Key. Must be a valid shell environment variable name (i.e. identifier).
                       If the value is invalid, the call is ignored.
            \param value Value. Must not contain control characters.
                       If the value contains control characters, only the portion leading up to it is written.

            This is expected to write data that may come from game configuration.
            The data is expected to be sane, but invalid data should not cause abort of a host run. */
        void addValue(const String_t& key, const String_t& value);

        /** Get accumulated content.
            \return content */
        afl::base::ConstBytes_t getContent() const;

     private:
        String_t m_buffer;
    };

} }

#endif
