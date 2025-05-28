/**
  *  \file test/util/nettest.cpp
  *  \brief Test for util::Net
  */

#include "util/net.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/net/http/simpledownloadlistener.hpp"
#include "afl/net/http/clientresponse.hpp"
#include "afl/net/http/clientrequest.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/base/memory.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/data/access.hpp"

using afl::net::http::SimpleDownloadListener;
using afl::net::http::ClientResponse;
using afl::net::http::ClientRequest;
using afl::string::NullTranslator;
using afl::base::ConstBytes_t;
using afl::test::LogListener;
using afl::data::Value;
using afl::data::Access;

namespace {
    void processData(afl::test::Assert a, SimpleDownloadListener& sdl, const char* data)
    {
        ClientResponse resp(false);
        ConstBytes_t dataBuffer = afl::string::toBytes(data);
        a.check("handleData", resp.handleData(dataBuffer));
        sdl.handleResponseHeader(resp);
        sdl.handleResponseData(0, dataBuffer);
    }
}

/*
 *  processDownloadResult
 */

AFL_TEST("util.net:processDownloadResult:success", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    processData(a, sdl, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n");
    sdl.handleSuccess();

    a.check("check", util::processDownloadResult("url", sdl, log, "logName", tx));
    a.checkEqual("log", log.getNumMessages(), 0U);
}

AFL_TEST("util.net:processDownloadResult:404", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    processData(a, sdl, "HTTP/1.0 404 Not found\r\nContent-Type: text/plain\r\n\r\nhello\n");
    sdl.handleSuccess();

    a.check("check", !util::processDownloadResult("url", sdl, log, "logName", tx));
    a.checkEqual("log", log.getNumMessages(), 1U);
}

AFL_TEST("util.net:processDownloadResult:failed", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    sdl.handleFailure(ClientRequest::Cancelled, "msg");

    a.check("check", !util::processDownloadResult("url", sdl, log, "logName", tx));
    a.checkEqual("log", log.getNumMessages(), 1U);
}

AFL_TEST("util.net:processDownloadResult:limit", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    sdl.setDataLimit(3);
    processData(a, sdl, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\nhello\n");
    sdl.handleSuccess();

    a.check("check", !util::processDownloadResult("url", sdl, log, "logName", tx));
    a.checkEqual("log", log.getNumMessages(), 1U);
    a.checkEqual("data", afl::string::fromBytes(sdl.getResponseData()), "hel");
}

/*
 *  processJSONResult
 */

AFL_TEST("util.net:processJSONResult:success", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    processData(a, sdl, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n{\"value\":42}\n");
    sdl.handleSuccess();

    std::auto_ptr<Value> result(util::processJSONResult("url", sdl, log, "logName", tx));
    a.checkNonNull("result", result.get());
    a.checkEqual("value", Access(result)("value").toInteger(), 42);
    a.checkEqual("log", log.getNumMessages(), 0U);
}

AFL_TEST("util.net:processJSONResult:404", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    processData(a, sdl, "HTTP/1.0 404 OK\r\nContent-Type: text/plain\r\n\r\n{\"value\":42}\n");
    sdl.handleSuccess();

    std::auto_ptr<Value> result(util::processJSONResult("url", sdl, log, "logName", tx));
    a.checkNull("result", result.get());
    a.checkEqual("log", log.getNumMessages(), 1U);
}

AFL_TEST("util.net:processJSONResult:parse-error", a)
{
    LogListener log;
    NullTranslator tx;
    SimpleDownloadListener sdl;
    processData(a, sdl, "HTTP/1.0 200 OK\r\nContent-Type: text/plain\r\n\r\n{\"value\":NONE}\n");
    sdl.handleSuccess();

    std::auto_ptr<Value> result(util::processJSONResult("url", sdl, log, "logName", tx));
    a.checkNull("result", result.get());
    a.checkEqual("log", log.getNumMessages(), 3U);
}
