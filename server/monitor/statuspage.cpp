/**
  *  \file server/monitor/statuspage.cpp
  */

#include "server/monitor/statuspage.hpp"
#include "afl/io/stream.hpp"
#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/parsedtime.hpp"
#include "afl/sys/time.hpp"
#include "server/monitor/status.hpp"

server::monitor::StatusPage::StatusPage(Status& st, afl::io::FileSystem& fs, String_t fileName)
    : m_status(st),
      m_fileSystem(fs),
      m_fileName(fileName)
{ }

server::monitor::StatusPage::~StatusPage()
{ }

bool
server::monitor::StatusPage::isValidMethod(const String_t& method) const
{
    return method == "GET";
}

bool
server::monitor::StatusPage::isValidPath() const
{
    return false;
}

void
server::monitor::StatusPage::handleRequest(afl::net::http::PageRequest& /*in*/, afl::net::http::PageResponse& out)
{
    // Read template into string
    afl::base::Ref<afl::io::Stream> file(m_fileSystem.openFile(m_fileName, afl::io::FileSystem::OpenRead));
    String_t content;
    uint8_t buffer[4096];
    while (size_t n = file->read(buffer)) {
        content.append((char*) buffer, n);
    }

    // Format template
    renderTemplate(content, out.body());

    // Headers
    out.setStatusCode(out.OK);
    out.headers().add("Server", "c2monitor");         // allow self-recognition in NetworkObserver
    out.headers().add("Content-Type", "text/html");
    out.headers().add("Pragma", "no-cache");          // defeat caching
    out.headers().add("Cache-Control", "no-cache");   // defeat caching
}

void
server::monitor::StatusPage::renderTemplate(const String_t& in, afl::io::DataSink& out)
{
    // ex planetscentral/monitor.cc:writeTemplate
    using afl::string::toBytes;
    afl::sys::Time statusTime;
    String_t statusText = m_status.render(statusTime);
    String_t historyText = m_status.renderTimeSeries();
    afl::sys::Time serverTime = afl::sys::Time::getCurrentTime();

    size_t pos = 0;
    size_t next;
    afl::base::ConstBytes_t stringAsBytes = toBytes(in);
    while ((next = in.find("$(", pos)) != String_t::npos) {
        out.handleFullData(stringAsBytes.subrange(pos, next - pos));
        if (in.compare(next, 9, "$(STATUS)") == 0) {
            out.handleFullData(toBytes(statusText));
            pos = next+9;
        } else if (in.compare(next, 10, "$(HISTORY)") == 0) {
            out.handleFullData(toBytes(historyText));
            pos = next+10;
        } else if (in.compare(next, 12, "$(CHECKTIME)") == 0) {
            afl::sys::ParsedTime pt;
            statusTime.unpack(pt, afl::sys::Time::UniversalTime);
            out.handleFullData(toBytes(pt.format("%d/%b/%Y %H:%M:%S GMT")));
            pos = next+12;
        } else if (in.compare(next, 13, "$(SERVERTIME)") == 0) {
            afl::sys::ParsedTime pt;
            serverTime.unpack(pt, afl::sys::Time::UniversalTime);
            out.handleFullData(toBytes(pt.format("%d/%b/%Y %H:%M:%S GMT")));
            pos = next+13;
        } else if (in.compare(next, 15, "$(CHECKTIMENUM)") == 0) {
            out.handleFullData(toBytes(afl::string::Format("%d", statusTime.getUnixTime())));
            pos = next+15;
        } else if (in.compare(next, 16, "$(SERVERTIMENUM)") == 0) {
            out.handleFullData(toBytes(afl::string::Format("%d", serverTime.getUnixTime())));
            pos = next+16;
        } else {
            out.handleFullData(toBytes("ERROR: unknown variable in template"));
            pos = next+1;
        }
    }
    out.handleFullData(stringAsBytes.subrange(pos));
}
