/**
  *  \file util/net.cpp
  *  \brief Network Utilities
  */

#include "util/net.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/string/format.hpp"

using afl::sys::LogListener;
using afl::string::Format;
using afl::net::http::SimpleDownloadListener;

bool
util::processDownloadResult(const String_t& url, afl::net::http::SimpleDownloadListener& listener, afl::sys::LogListener& log, const char* logName, afl::string::Translator& tx)
{
    switch (listener.wait()) {
     case SimpleDownloadListener::Succeeded:
        if (listener.getStatusCode() != 200) {
            log.write(LogListener::Error, logName, Format(tx("%s: network access failed (HTTP error %d)"), url, listener.getStatusCode()));
            return false;
        }
        break;
     case SimpleDownloadListener::Failed:
        log.write(LogListener::Error, logName, Format(tx("%s: network access failed (%s)"), url, toString(listener.getFailureReason())));
        return false;
     case SimpleDownloadListener::TimedOut:
        // Cannot happen because we do not use wait() with timeout.
        log.write(LogListener::Error, logName, Format(tx("%s: network access timed out"), url));
        return false;
     case SimpleDownloadListener::LimitExceeded:
        log.write(LogListener::Error, logName, Format(tx("%s: network access exceeded limit"), url));
        return false;
    }
    return true;
}

std::auto_ptr<afl::data::Value>
util::processJSONResult(const String_t& url, afl::net::http::SimpleDownloadListener& listener, afl::sys::LogListener& log, const char* logName, afl::string::Translator& tx)
{
    if (!processDownloadResult(url, listener, log, logName, tx)) {
        return std::auto_ptr<afl::data::Value>();
    }

    // Parse JSON
    // Note that compilation of this code seems to be brittle with gcc-12.2.0.
    // In the original version, with processDownloadResult() and processJSONResult() in one function, there's two problems:
    // - The 'return std::auto_ptr<..>(Parser(...).parseComplete())' statement crashes with SIGSEGV if parseComplete() throws.
    //   This can be worked around by assigning to a local variable.
    // - For the fixed code, compiler complains about an unused, nameless variable at the end of the 'try',
    //   supposedly defined in the middle of processDownloadResult()'s switch.
    //   This can be worked around by wrapping the code below in braces.
    afl::data::DefaultValueFactory factory;
    afl::io::ConstMemoryStream cms(listener.getResponseData());
    afl::io::BufferedStream buf(cms);
    try {
        return std::auto_ptr<afl::data::Value>(afl::io::json::Parser(buf, factory).parseComplete());
    }
    catch (std::exception& e) {
        log.write(LogListener::Error, logName, Format(tx("%s: received invalid data from network"), url));
        log.write(LogListener::Info,  logName, tx("Parse error"), e);

        // Log failing fragment
        afl::io::Stream::FileSize_t pos = buf.getPos();
        if (pos > 0) {
            --pos;
            buf.setPos(pos);
        }
        uint8_t tmp[30];
        afl::base::Bytes_t bytes(tmp);
        bytes.trim(buf.read(bytes));

        log.write(LogListener::Trace, logName, Format("at byte %d, \"%s\"", pos, afl::string::fromBytes(bytes)));
    }
    return std::auto_ptr<afl::data::Value>();
}
