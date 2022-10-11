/**
  *  \file game/parser/binarytransfer.hpp
  *  \brief Binary Message Transfer (VPA Data Transmission)
  */
#ifndef C2NG_GAME_PARSER_BINARYTRANSFER_HPP
#define C2NG_GAME_PARSER_BINARYTRANSFER_HPP

#include <utility>
#include "afl/base/memory.hpp"
#include "afl/charset/charset.hpp"
#include "afl/string/string.hpp"
#include "game/hostversion.hpp"
#include "game/map/drawing.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"

namespace game { namespace parser {

    /*
     *  Encoding
     */

    /** Pack a planet into a binary message.

        The planet can be received by VPA and PCC/PCC2.
        The timestamp differences are lost in the transfer.

        @param pl   Planet
        @param cs   Game character set
        @param host Host version (for industry levels)
        @return Multi-line string containing the encoded data */
    String_t packBinaryPlanet(const game::map::Planet& pl, afl::charset::Charset& cs, const HostVersion& host);

    /** Pack a minefield into a binary message.

        The minefield can be received by VPA and PCC/PCC2.

        @param mf   Minefield
        @return Multi-line string containing the encoded data */
    String_t packBinaryMinefield(const game::map::Minefield& mf);

    /** Pack a drawing into a binary message.

        The drawing can be received by VPA and PCC2.
        A drawing sent by PCC2 is received unchanged by PCC2.
        Drawings sent between VPA and PCC lose precision:
        - marker shapes and colors are reproduced only approximately
        - a VPA "dotted line" is received as PCC2 "rectangle"

        @param d    Drawing
        @param cs   Game character set
        @return Multi-line string containing the encoded data */
    String_t packBinaryDrawing(const game::map::Drawing& d, afl::charset::Charset& cs);


    /*
     *  Decoding
     */

    /** Unpacker result. */
    enum UnpackResult {
        /** Data correctly unpacked. */
        UnpackSuccess,

        /** Message does not contain encoded data (or obvious syntax error). */
        UnpackUnspecial,

        /** Message does contain encoded data, but it cannot be decoded.
            The choice between this one and UnpackUnspecial is not perfectly objective. */
        UnpackFailed,

        /** Message does contain encoded data, but the checksum check failed. */
        UnpackChecksumError
    };


    /** Message type. */
    enum MessageType {
        NoMessage,              ///< No message or unknown type (used with a failure code).
        MinefieldMessage,       ///< Single minefield (VPA "Minefield xx").
        PlanetMessage,          ///< Single planet (VPA "Planet xx").
        DrawingMessage,         ///< Single drawing (VPA "Marker").
        StatisticMessage        ///< Statistic summary (VPA "Statistic Txx").
    };

    /** Result of unpackBinaryMessage(). */
    typedef std::pair<UnpackResult,MessageType> UnpackResultPair_t;


    /** Try to unpack a binary message.

        @param [in]  in      Message, split into lines (see splitMessage())
        @param [in]  turnNr  Turn number.
                             Pass turnNr-1 if this is a message received through inbox (in this case, data is from previous turn).
                             Pass turnNr if this is a message from a file (in this case, data was probably transferred manually).
        @param [out] info    Parse result. Information is added here if the result is UnpackSuccess.
        @param [in]  cs      Game character set
        @return pair of UnpackResult and MessageType. MessageType is guaranteed only for UnpackSuccess. */
    UnpackResultPair_t unpackBinaryMessage(afl::base::Memory<const String_t> in, int turnNr, afl::container::PtrVector<MessageInformation>& info, afl::charset::Charset& cs);

} }

#endif
