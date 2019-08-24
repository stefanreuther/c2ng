/**
  *  \file game/v3/udata/reader.hpp
  */
#ifndef C2NG_GAME_V3_UDATA_READER_HPP
#define C2NG_GAME_V3_UDATA_READER_HPP

#include "afl/base/deletable.hpp"
#include "afl/io/stream.hpp"
#include "game/timestamp.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"

namespace game { namespace v3 { namespace udata {

    /** UTIL.DAT parser.

        This encapsulates the logic for reading UTILx.DAT.
        Derived classes override handleRecord() to actually process the records. */
    class Reader {
     public:
        /** Constructor. */
        Reader();

        /** Read UTIL.DAT.
            Reads the file and calls process() for each record, in order.
            \param in File to read */
        void read(afl::io::Stream& in);

        /** Check whether this is a valid UTILx.DAT.
            Valid files start with a valid control record.
            Optionally, return the timestamp in *ts.
            This does not modify the file position (that is, it restores the position after reading)
            so it can safely be called before process() on the same stream.

            \param in File to read
            \param ts If non-null, the timestamp is reported here

            \returns true iff file seems valid.
            \pre stream is seekable.
            \throws never, except when the stream throws. */
        static bool check(afl::io::Stream& in, Timestamp* ts);

        /** Process one record. Can throw an exception to abort parsing.
            \param recordId Record Id
            \param data     Data
            \return true to continue parsing, false to stop. */
        // ex GUtilReader::process()
        virtual bool handleRecord(uint16_t recordId, afl::base::ConstBytes_t data) = 0;

        /** Handle error. Reports a file format error (i.e. file too short). */
        // ex GUtilReader::failure
        virtual void handleError(afl::io::Stream& in) = 0;

        /** Handle end of file.
            Called after the last handleRecord().
            This method can perform cleanup tasks. */
        virtual void handleEnd() = 0;
    };

} } }

#endif
