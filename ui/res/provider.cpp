/**
  *  \file ui/res/provider.cpp
  */

#include "ui/res/provider.hpp"

// /** Open a resource file. If the specified file name ends with a dot, this
//     searches for a file according to the suffix list. Otherwise, only the
//     exact name specified is attempted.

//     \param dir      Directory to look in
//     \param filename (User-)specified file name
//     \param suffixes suffix list, null-terminated
//     \return stream, or null */
afl::base::Ptr<afl::io::Stream>
ui::res::Provider::openResourceFile(afl::io::Directory& dir, String_t fileName, afl::base::Memory<const char*const> suffixes)
{
    // ex resmgr/resmgr.h:openResourceFile
    if (fileName.size() != 0 && fileName[fileName.size()-1] == '.') {
        // File name has the form "xxx.", which menas: look with suffixes
        while (const char*const* pp = suffixes.eat()) {
            if (const char* p = *pp) {
                afl::base::Ptr<afl::io::Stream> s = dir.openFileNT(fileName + p, afl::io::FileSystem::OpenRead);
                if (s.get() != 0) {
                    return s;
                }
            }
        }
    }

    // Nothing found or no suffix search requested: try opening as-is
    return dir.openFileNT(fileName, afl::io::FileSystem::OpenRead);
}

afl::base::Memory<const char*const>
ui::res::Provider::graphicsSuffixes()
{
    // ex resmgr/resmgr.h:gfx_suffix_list
    static const char* const list[] = { "cd", "gfx", "bmp", "png", "jpg" };

    // Note that this cast is required by gcc < 6.1, but not by clang, icc, or gcc 6.1+
    return afl::base::Memory<const char*const>(list);
}
