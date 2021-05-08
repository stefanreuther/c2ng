/**
  *  \file ui/res/factory.cpp
  */

#include <cstring>
#include "ui/res/factory.hpp"
#include "afl/string/string.hpp"
#include "ui/res/directoryprovider.hpp"
#include "ui/res/resourcefileprovider.hpp"
#include "ui/res/winplanbitmapprovider.hpp"
#include "ui/res/winplanvcrprovider.hpp"

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
ui::res::createProvider(String_t name, String_t baseDirectory, afl::io::FileSystem& fs, afl::sys::LogListener& log, afl::string::Translator& tx)
{
    if (hasPrefix(name, "dir")) {
        return new DirectoryProvider(fs.openDirectory(fs.makePathName(baseDirectory, name)), fs, log, tx);
    } else if (hasPrefix(name, "wp")) {
        return new WinplanBitmapProvider(fs.openDirectory(fs.makePathName(baseDirectory, name)));
    } else if (hasPrefix(name, "wpvcr")) {
        return new WinplanVcrProvider(fs.openFile(fs.makePathName(baseDirectory, name), fs.OpenRead));
    } else {
        // Trim optional "res:" prefix
        hasPrefix(name, "res");
        return new ResourceFileProvider(fs.openFile(fs.makePathName(baseDirectory, name), fs.OpenRead));
    }
}
