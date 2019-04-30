/**
  *  \file server/talk/newsrc.cpp
  *  \brief Class server::talk::Newsrc
  *
  *  user:$UID:forum:newsrc:data : hash
  *  user:$UID:forum:newsrc:index : int
  *          This represents a bitset.
  *          The bitset is split into lines of 1024 bytes = 8192 bits.
  *          Hash key is the line number, starting at 0.
  *          Each bit is 1 if the message is already read.
  *          Compaction:
  *          - treat all lines < index as "all-1"
  *          - otherwise, look into the hash
  *          - if line is missing, treat as "all-0"
  *
  *  FIXME: this needs a little love; it has been taken over verbatim.
  */

#include "server/talk/newsrc.hpp"
#include "afl/string/format.hpp"
#include "afl/net/redis/field.hpp"
#include "afl/net/redis/stringfield.hpp"

namespace {
    enum {
        LineShift = 13,
        LineSize  = 1 << LineShift,
        LineMask  = LineSize - 1,

        LineBytes = LineSize / 8
    };

    String_t itoa(int32_t n)
    {
        return afl::string::Format("%d", n);
    }
}

server::talk::Newsrc::Newsrc(afl::net::redis::Subtree root)
    : root(root),
      readAllBelowLine(index().get()),
      cache(),
      cacheIndex(-1),
      cacheDirty(false)
{
    // ex Newsrc::Newsrc
}
    
void
server::talk::Newsrc::save()
{
    // ex Newsrc::save
    // Remove entirely-read lines.
    while (cacheIndex == readAllBelowLine && cache.find_first_not_of(char(0xFF)) == String_t::npos) {
        // This line is entirely read, remove it.
        data().field(itoa(cacheIndex)).remove();
        ++readAllBelowLine;
        index().set(readAllBelowLine);

        // Load next one.
        doLoad(cacheIndex+1);
    }

    // If a dirty line remains, save it
    if (cacheDirty) {
        if (cache.find_first_not_of(char(0)) == String_t::npos) {
            data().field(itoa(cacheIndex)).remove();
        } else {
            data().stringField(itoa(cacheIndex)).set(cache);
            cacheDirty = false;
        }
    }
}

bool
server::talk::Newsrc::get(int32_t messageId)
{
    // ex Newsrc::get
    // Split into line/column
    int32_t line = messageId >> LineShift;
    int32_t column = messageId & LineMask;

    // Quick handling?
    if (line < readAllBelowLine) {
        return true;
    }

    // Use cache
    loadCache(line);
    int32_t byte = column >> 3;
    int32_t bit = (column & 7);
    return ((uint8_t(cache[byte]) & (1 << bit)) != 0);
}

void
server::talk::Newsrc::set(int32_t messageId)
{
    // ex Newsrc::set
    // Split into line/column
    int32_t line = messageId >> LineShift;
    int32_t column = messageId & LineMask;

    // Anything to do?
    if (line >= readAllBelowLine) {
        // Use cache
        loadCache(line);
        int32_t byte = column >> 3;
        int32_t bit = (column & 7);
        uint8_t mask = uint8_t(1 << bit);
        if ((uint8_t(cache[byte]) & mask) == 0) {
            cache[byte] = uint8_t(cache[byte] | mask);
            cacheDirty = true;
        }
    }
}

void
server::talk::Newsrc::clear(int32_t messageId)
{
    // ex Newsrc::clear
    // Split into line/column
    int32_t line = messageId >> LineShift;
    int32_t column = messageId & LineMask;

    // Is the line we need actually available?
    while (line < readAllBelowLine) {
        // No, we have to create a new all-FF line
        save();
        --readAllBelowLine;
        cache.assign(size_t(LineSize), char(0xFF));
        cacheIndex = readAllBelowLine;
        cacheDirty = true;
        index().set(readAllBelowLine);
    }

    // Regular operation through cache
    loadCache(line);
    int32_t byte = column >> 3;
    int32_t bit = (column & 7);
    uint8_t mask = uint8_t(1 << bit);
    if ((uint8_t(cache[byte]) & mask) != 0) {
        cache[byte] = uint8_t(cache[byte] & ~mask);
        cacheDirty = true;
    }
}

afl::net::redis::IntegerKey
server::talk::Newsrc::index()
{
    // ex Newsrc::index
    return root.intKey("index");
}

afl::net::redis::HashKey
server::talk::Newsrc::data()
{
    // ex Newsrc::data
    return root.hashKey("data");
}

void
server::talk::Newsrc::loadCache(int32_t index)
{
    // ex Newsrc::loadCache
    if (index != cacheIndex) {
        // Save old value
        save();
    }
    if (index != cacheIndex) {
        // Load new value. 'save' could have caused the new line to be loaded
        // already.
        doLoad(index);
    }
}

void
server::talk::Newsrc::doLoad(int32_t index)
{
    // ex Newsrc::doLoad
    cache      = data().stringField(itoa(index)).get();
    cacheIndex = index;
    cacheDirty = false;

    // Make cache value canonical
    if (cache.size() > LineBytes) {
        cache.erase(LineBytes);
    }
    if (cache.size() < LineBytes) {
        cache.append(LineBytes - cache.size(), char(0));
    }
}
