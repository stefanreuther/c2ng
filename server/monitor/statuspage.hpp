/**
  *  \file server/monitor/statuspage.hpp
  */
#ifndef C2NG_SERVER_MONITOR_STATUSPAGE_HPP
#define C2NG_SERVER_MONITOR_STATUSPAGE_HPP

#include "afl/net/http/page.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/datasink.hpp"

namespace server { namespace monitor {

    class Status;

    class StatusPage : public afl::net::http::Page {
     public:
        StatusPage(Status& st, afl::io::FileSystem& fs, String_t fileName);
        ~StatusPage();
        virtual bool isValidMethod(const String_t& method) const;
        virtual bool isValidPath() const; 
        virtual void handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out);

     private:
        Status& m_status;
        afl::io::FileSystem& m_fileSystem;
        String_t m_fileName;

        void renderTemplate(const String_t& in, afl::io::DataSink& out);
    };

} }

#endif
