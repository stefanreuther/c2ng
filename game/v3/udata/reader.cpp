/**
  *  \file game/v3/udata/reader.cpp
  */

#include "game/v3/udata/reader.hpp"
#include "afl/base/growablememory.hpp"
#include "game/v3/structures.hpp"

namespace gt = game::v3::structures;

game::v3::udata::Reader::Reader()
{ }

// Read UTIL.DAT.
void
game::v3::udata::Reader::read(afl::io::Stream& in)
{
    // ex GUtilReader::read
    gt::UtilChunkHeader header;
    while (in.read(afl::base::fromObject(header)) == sizeof(header)) {
        size_t recordSize = header.recordSize;
        uint16_t recordType = header.recordType;
        afl::base::GrowableBytes_t buffer;
        if (recordSize != 0) {
            buffer.resize(recordSize);
            if (in.read(buffer) != recordSize) {
                handleError(in);
                break;
            }
        }
        if (!handleRecord(recordType, buffer)) {
            break;
        }
    }
}

// Check whether this is a valid UTILx.DAT.
bool
game::v3::udata::Reader::check(afl::io::Stream& in, Timestamp* ts)
{
    // ex GUtilReader::check
    afl::io::Stream::FileSize_t pos = in.getPos();
    gt::UtilChunkHeader header;
    gt::Util13ControlMinimal data;

    bool ok = (in.read(afl::base::fromObject(header)) != sizeof(header)
               || header.recordType != gt::UTIL_CONTROL_ID
               || header.recordSize < sizeof(data)
               || header.recordSize > 1024 /* arbitrary, but attempt to reject text files */
               || in.read(afl::base::fromObject(data)) != sizeof(data));

    in.setPos(pos);
    if (ok && ts != 0) {
        *ts = data.timestamp;
    }
    return ok;
}
