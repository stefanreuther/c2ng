/**
  *  \file server/interface/hostspecification.hpp
  *  \brief Interface server::interface::HostSpecification
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSPECIFICATION_HPP
#define C2NG_SERVER_INTERFACE_HOSTSPECIFICATION_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "afl/data/stringlist.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host specification access.
        This interface allows to retrieve specification data. */
    class HostSpecification : afl::base::Deletable {
     public:
        /** Result format selection. */
        enum Format {
            /** Return value directly.
                Returned value is a HashValue with the keys,
                each containing a VectorValue or HashValue as appropriate. */
            Direct,

            /** Return stringified JSON.
                Returned value is a JSON string.
                Use if data is given to a JSON consumer without further inspection. */
            JsonString
        };

        /** Get data for a shiplist (SPECSHIPLIST).
            @param shiplistId   Shiplist Id
            @param format       Desired format
            @param keys         Keys to retrieve
            @return newly-allocated value */
        virtual Value_t* getShiplistData(String_t shiplistId, Format format, const afl::data::StringList_t& keys) = 0;

        /** Get data for a shiplist (SPECGAME).
            @param gameId       Game Id
            @param format       Desired format
            @param keys         Keys to retrieve
            @return newly-allocated value */
        virtual Value_t* getGameData(int32_t gameId, Format format, const afl::data::StringList_t& keys) = 0;

        /** Convert Format to string.
            @param fmt Value to format
            @return formatted value */
        static String_t formatFormat(Format fmt);

        /** Parse string to Format.
            @param str String to parse
            @return Format, if parsed successfully */
        static afl::base::Optional<Format> parseFormat(const String_t& str);
    };

} }

#endif
