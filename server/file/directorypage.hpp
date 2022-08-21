/**
  *  \file server/file/directorypage.hpp
  *  \brief Class server::file::DirectoryPage
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYPAGE_HPP
#define C2NG_SERVER_FILE_DIRECTORYPAGE_HPP

#include "afl/net/http/page.hpp"

namespace server { namespace file {

    class ReadOnlyDirectoryHandler;

    /** HTTP Page implementation for serving a DirectoryHandler.
        Add this to a afl::net::http::PageDispatcher to serve a DirectoryHandler via HTTP.

        This is intended for testing (c2fileclient serve).
        Therefore, this class does not implement cache management on either side:
        all data is always re-read from the DirectoryHandler, not cached,
        and no cache negotiation with the web browser is done.

        Likewise, it does not implement any access control. */
    class DirectoryPage : public afl::net::http::Page {
     public:
        /** Constructor.
            \param dh DirectoryHandler */
        explicit DirectoryPage(ReadOnlyDirectoryHandler& dh);

        virtual bool isValidMethod(const String_t& method) const;
        virtual bool isValidPath() const;
        virtual void handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out);
     private:
        class Sorter;
        ReadOnlyDirectoryHandler& m_directoryHandler;
    };

} }

#endif
