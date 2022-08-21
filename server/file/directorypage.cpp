/**
  *  \file server/file/directorypage.cpp
  *  \brief Class server::file::DirectoryPage
  */

#include <algorithm>
#include "server/file/directorypage.hpp"
#include "afl/net/http/pagerequest.hpp"
#include "afl/net/http/pageresponse.hpp"
#include "afl/string/format.hpp"
#include "server/file/readonlydirectoryhandler.hpp"
#include "server/file/utils.hpp"
#include "util/string.hpp"

using afl::string::Format;
using afl::string::toBytes;

namespace {
    String_t getMimeType(String_t basename)
    {
        // FIXME: duplicated from server::mailout::Template
        String_t::size_type i = basename.rfind('.');
        if (i < basename.size()) {
            /* This is the same repertoire as in file.cgi as of 02/Apr/2012.
               Added .pl, .sh, dotfiles for c2fileclient. */
            String_t ext = afl::string::strLCase(basename.substr(i+1));
            if (i == 0 || ext == "ini" || ext == "src" || ext == "txt" || ext == "cfg" || ext == "log" || ext == "q" || ext == "frag" || ext == "sh" || ext == "pl") {
                return "text/plain; charset=ISO-8859-1";
            } else if (ext == "html" || ext == "htm") {
                return "text/html";
            } else if (ext == "png") {
                return "image/png";
            } else if (ext == "gif") {
                return "image/gif";
            } else if (ext == "jpg" || ext == "jpeg") {
                return "image/jpeg";
            } else if (ext == "bmp") {
                return "image/bmp";
            } else if (ext == "zip") {
                return "application/zip";
            } else {
                return "application/octet-stream";
            }
        } else {
            return "application/octet-stream";
        }
    }
}


class server::file::DirectoryPage::Sorter {
 public:
    bool operator()(const DirectoryHandler::Info& a, const DirectoryHandler::Info& b) const
        {
            if (a.type != b.type) {
                return a.type > b.type;
            }
            return a.name < b.name;
        }
};

server::file::DirectoryPage::DirectoryPage(ReadOnlyDirectoryHandler& dh)
    : m_directoryHandler(dh)
{ }

bool
server::file::DirectoryPage::isValidMethod(const String_t& method) const
{
    return method == "GET";
}

bool
server::file::DirectoryPage::isValidPath() const
{
    return true;
}

void
server::file::DirectoryPage::handleRequest(afl::net::http::PageRequest& in, afl::net::http::PageResponse& out)
{
    // Parse path. Each component must be a directory.
    const String_t& path = in.getPath();
    size_t pos = std::min(path.find_first_not_of("/"), path.size());
    ReadOnlyDirectoryHandler* dir = &m_directoryHandler;
    size_t n;
    while ((n = path.find('/', pos)) != String_t::npos) {
        DirectoryHandler::Info info;
        if (!dir->findItem(String_t(path, pos, n-pos), info) || info.type != DirectoryHandler::IsDirectory) {
            out.setStatusCode(out.NOT_FOUND);
            return;
        }
        dir = dir->getDirectory(info);
        if (!dir) {
            out.setStatusCode(out.INTERNAL_SERVER_ERROR);
            return;
        }
        pos = n+1;
    }

    // Process result
    if (pos == path.size()) {
        // Last component is empty, i.e. path of the form "/foo/": directory
        out.headers().set("Content-Type", "text/html; charset=utf-8");
        out.body().handleFullData(toBytes(Format("<html><head><title>c2file: %s</title><style>a:link{text-decoration:none}</style><body><h1>c2file: %$s</h1><pre>"
                                                 "<u>Type Content Id                               Size        Name</u>\n",
                                                 util::encodeHtml(in.getPath(), false))));

        InfoVector_t children;
        listDirectory(children, *dir);
        std::sort(children.begin(), children.end(), Sorter());

        if (path.size() > 1) {
            out.body().handleFullData(toBytes("UP   -                                                 -  <a href=\"../\">(parent)</a>\n"));
        }

        for (size_t i = 0, n = children.size(); i < n; ++i) {
            const DirectoryHandler::Info& ch = children[i];

            String_t size = "-";
            if (const int32_t* p = ch.size.get()) {
                size = afl::string::Format("%d", *p);
            }
            String_t type = "?";
            String_t suffix = "";
            switch (ch.type) {
             case DirectoryHandler::IsUnknown:   type = "UNK";                break;
             case DirectoryHandler::IsDirectory: type = "DIR";  suffix = "/"; break;
             case DirectoryHandler::IsFile:      type = "FILE";               break;
            }
            out.body().handleFullData(toBytes(Format("%-4s %-40s %10s  <a href=\"%s%s\">%3$s</a>\n")
                                              << type << ch.contentId.orElse("-") << size
                                              << util::encodeHtml(ch.name, false) << suffix));
        }
        out.body().handleFullData(toBytes("</pre></html>\n"));
    } else {
        // Not empty last component
        DirectoryHandler::Info info;
        if (!dir->findItem(String_t(path, pos), info)) {
            out.setStatusCode(out.NOT_FOUND);
            return;
        }

        switch (info.type) {
         case DirectoryHandler::IsUnknown:
            out.headers().set("Content-Type", "text/plain; charset=utf-8");
            out.body().handleFullData(toBytes("Not renderable"));
            break;

         case DirectoryHandler::IsFile:
            out.headers().set("Content-Type", getMimeType(info.name));
            out.body().handleFullData(dir->getFile(info)->get());
            break;

         case DirectoryHandler::IsDirectory:
            out.setRedirect(in.getRootPath() + in.getSelfPath() + in.getPath() + "/");
            break;
        }
    }
}
