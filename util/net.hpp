/**
  *  \file util/net.hpp
  *  \brief Network Utilities
  */
#ifndef C2NG_UTIL_NET_HPP
#define C2NG_UTIL_NET_HPP

#include <memory>
#include "afl/data/value.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace util {

    /** Process download result.
        Calls listener.wait() and inspects the result.
        If the result indicates success, returns true.
        If the result indicates failure, logs an error message and returns false.

        @param url         URL (for log messages)
        @param listener    DownloadListener; initialized using afl::net::http::Manager::getFile() or afl::net::http::Manager::postFile()
        @param log         Logger
        @param logName     Log channel name
        @param tx          Translator

        @return true if download succeeded */
    bool processDownloadResult(const String_t& url, afl::net::http::SimpleDownloadListener& listener, afl::sys::LogListener& log, const char* logName, afl::string::Translator& tx);

    /** Process JSON download result.
        Completes the download as per processDownloadResult().
        On success, tries to interpret the response as JSON.
        If that succeeds, returns the parsed object tree.
        On any error, returns a null object.

        @param url         URL (for log messages)
        @param listener    DownloadListener; initialized using afl::net::http::Manager::getFile() or afl::net::http::Manager::postFile()
        @param log         Logger
        @param logName     Log channel name
        @param tx          Translator

        @return newly-allocated object tree */
    std::auto_ptr<afl::data::Value> processJSONResult(const String_t& url, afl::net::http::SimpleDownloadListener& listener, afl::sys::LogListener& log, const char* logName, afl::string::Translator& tx);

}

#endif
