/**
  *  \file ui/res/factory.cpp
  */

#include <cstring>
#include "ui/res/factory.hpp"
#include "afl/string/string.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/resourcefileprovider.hpp"
#include "ui/res/winplanbitmapprovider.hpp"

namespace {
    bool hasPrefix(String_t& name, const char* pfx)
    {
        size_t n = std::strlen(pfx);
        if (name.size() > n && name[n] == ':' && afl::string::strCaseCompare(name.substr(0, n), pfx) == 0) {
            name.erase(0, n+1);
            return true;
        } else {
            return false;
        }
    }
}

ui::res::Provider*
ui::res::createProvider(String_t name, String_t baseDirectory, afl::io::FileSystem& fs)
{
    if (hasPrefix(name, "dir")) {
        return new DirectoryProvider(fs.openDirectory(fs.makePathName(baseDirectory, name)));
    } else if (hasPrefix(name, "wp")) {
        return new WinplanBitmapProvider(fs.openDirectory(fs.makePathName(baseDirectory, name)));
    } else {
        // Trim optional "res:" prefix
        hasPrefix(name, "res");
        return new ResourceFileProvider(fs.openFile(fs.makePathName(baseDirectory, name), fs.OpenRead));
    }
}


// FIXME: finish
// static const ResProviderElem providers[] = {
//     { "res",   &genResProvider<ResProviderResFile> },
//     { "wp",    &genResProvider<ResProviderWinplanBitmaps> },
//     { "wpvcr", &genResProvider<ResProviderWinplanVcr> },
//     { "dir",   &genResDirectory }
// };
