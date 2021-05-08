/**
  *  \file server/common/randomidgenerator.cpp
  *  \brief Class server::common::RandomIdGenerator
  */

#include "server/common/randomidgenerator.hpp"
#include "afl/base/ptr.hpp"
#include "afl/bits/uint64le.hpp"
#include "afl/bits/value.hpp"
#include "afl/checksums/sha1.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/time.hpp"

server::common::RandomIdGenerator::RandomIdGenerator(afl::io::FileSystem& fs)
{
    init(fs);
}

server::common::RandomIdGenerator::~RandomIdGenerator()
{ }

String_t
server::common::RandomIdGenerator::createId()
{
    // Advance
    size_t i = 0;
    while (uint8_t* p = m_state.at(i)) {
        ++*p;
        if (*p != 0) {
            break;
        }
        ++i;
    }

    // Compute hash
    afl::checksums::SHA1 hash;
    hash.add(m_state);
    return hash.getHashAsHexString();
}

void
server::common::RandomIdGenerator::init(afl::io::FileSystem& fs)
{
    m_state.clear();

    // Obtain some random data.
    // FIXME: add Windows version? Make a nicer porting interface?
    afl::base::Ptr<afl::io::Stream> rng = fs.openFileNT("/dev/urandom", afl::io::FileSystem::OpenRead);
    if (rng.get() != 0) {
        uint8_t data[32];
        rng->fullRead(data);
        m_state.append(data);
    }

    // Add time as additional entropy (and fallback if there is no system RNG)
    afl::bits::Value<afl::bits::UInt64LE> time;
    time = afl::sys::Time::getCurrentTime().getRepresentation();
    m_state.append(time.m_bytes);
}
