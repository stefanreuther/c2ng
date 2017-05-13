/**
  *  \file server/file/utils.cpp
  *  \brief server::file Utilities
  */

#include <memory>
#include "server/file/utils.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "server/errors.hpp"
#include "server/file/directoryhandler.hpp"

using server::file::DirectoryHandler;

namespace {
    /** Comparator for DirectoryHandler::Info; sort by name. */
    struct CompareListItems {
        bool operator()(const DirectoryHandler::Info& a, const DirectoryHandler::Info& b)
            { return a.name < b.name; }
    };

    /** Copy a file.
        \param out Output directory
        \param in Input directory
        \param inChild Input file info */
    void copyFile(DirectoryHandler& out, DirectoryHandler& in, const DirectoryHandler::Info& inChild)
    {
        if (!out.copyFile(in, inChild, inChild.name).isValid()) {
            out.createFile(inChild.name, in.getFile(inChild)->get());
        }
    }

    /** Copy child.
        Copies files or directories.
        Requires that the target does not exist (or, that we're overwriting a file with a file).
        \param out Output directory
        \param in Input directory
        \param inChild Input item info */
    void copyChild(DirectoryHandler& out, DirectoryHandler& in, const DirectoryHandler::Info& inChild)
    {
        switch (inChild.type) {
         case DirectoryHandler::IsUnknown:
            // Ignore unknown
            break;

         case DirectoryHandler::IsFile:
            copyFile(out, in, inChild);
            break;

         case DirectoryHandler::IsDirectory: {
            std::auto_ptr<DirectoryHandler> outHandler(out.getDirectory(out.createDirectory(inChild.name)));
            std::auto_ptr<DirectoryHandler> inHandler(in.getDirectory(inChild));
            copyDirectory(*outHandler, *inHandler, true);
            break;
         }
        }
    }

    /** Remove a child.
        \param dir Directory
        \param child Child to remove */
    void removeChild(DirectoryHandler& dir, const DirectoryHandler::Info& child)
    {
        switch (child.type) {
         case DirectoryHandler::IsUnknown:
            // Ignore unknown
            break;

         case DirectoryHandler::IsFile:
            // Erase file
            dir.removeFile(child.name);
            break;

         case DirectoryHandler::IsDirectory: {
            // Recursively erase directory
            std::auto_ptr<DirectoryHandler> subdir(dir.getDirectory(child));
            removeDirectoryContent(*subdir);
            dir.removeDirectory(child.name);
            break;
         }
        }
    }
}


// List a directory.
void
server::file::listDirectory(InfoVector_t& out, DirectoryHandler& dir)
{
    // Read input content
    class Callback : public DirectoryHandler::Callback {
     public:
        Callback(InfoVector_t& out)
            : m_out(out)
            { }

        virtual void addItem(const DirectoryHandler::Info& info)
            { m_out.push_back(info); }

     private:
        InfoVector_t& m_out;
    };
    Callback cb(out);
    dir.readContent(cb);
}

// Copy a directory or directory tree.
void
server::file::copyDirectory(DirectoryHandler& out, DirectoryHandler& in, bool recursive)
{
    // Read content
    InfoVector_t inChildren;
    listDirectory(inChildren, in);

    InfoVector_t outChildren;
    listDirectory(outChildren, out);

    // Process it
    for (size_t i = 0, n = inChildren.size(); i < n; ++i) {
        const DirectoryHandler::Info& ch = inChildren[i];
        switch (ch.type) {
         case DirectoryHandler::IsUnknown:
            // Ignore unknown
            break;

         case DirectoryHandler::IsFile:
            // Copy file
            copyFile(out, in, ch);
            break;

         case DirectoryHandler::IsDirectory:
            // Recursively copy directory if desired
            if (recursive) {
                const DirectoryHandler::Info* outInfo = 0;
                for (size_t j = 0, n = outChildren.size(); j < n; ++j) {
                    if (outChildren[j].name == ch.name) {
                        outInfo = &outChildren[j];
                        break;
                    }
                }

                // Handler for output
                std::auto_ptr<DirectoryHandler> outHandler;
                if (outInfo != 0) {
                    if (outInfo->type != DirectoryHandler::IsDirectory) {
                        throw afl::except::FileProblemException(ch.name, NOT_A_DIRECTORY);
                    }
                    outHandler.reset(out.getDirectory(*outInfo));
                } else {
                    outHandler.reset(out.getDirectory(out.createDirectory(ch.name)));
                }

                // Handler for input
                std::auto_ptr<DirectoryHandler> inHandler(in.getDirectory(ch));
                copyDirectory(*outHandler, *inHandler, recursive);
            }
            break;
        }
    }
}

// Remove a directory's content.
void
server::file::removeDirectoryContent(DirectoryHandler& dir)
{
    // Read content
    InfoVector_t children;
    listDirectory(children, dir);

    // Process it
    for (size_t i = 0, n = children.size(); i < n; ++i) {
        removeChild(dir, children[i]);
    }
}

// Synchronize a directory tree.
void
server::file::synchronizeDirectories(DirectoryHandler& out, DirectoryHandler& in)
{
    // Read content
    InfoVector_t inChildren;
    listDirectory(inChildren, in);
    std::sort(inChildren.begin(), inChildren.end(), CompareListItems());

    InfoVector_t outChildren;
    listDirectory(outChildren, out);
    std::sort(outChildren.begin(), outChildren.end(), CompareListItems());

    // Process it
    size_t inIndex = 0;
    size_t outIndex = 0;
    while (inIndex < inChildren.size() && outIndex < outChildren.size()) {
        const DirectoryHandler::Info& inChild = inChildren[inIndex];
        const DirectoryHandler::Info& outChild = outChildren[outIndex];
        if (inChild.name < outChild.name) {
            // inChild comes first -> copy
            copyChild(out, in, inChild);
            ++inIndex;
        } else if (inChild.name > outChild.name) {
            // outChild comes first -> delete
            removeChild(out, outChild);
            ++outIndex;
        } else {
            // Same name -> check for overwrite
            if (inChild.type != outChild.type) {
                removeChild(out, outChild);
                copyChild(out, in, inChild);
            } else {
                switch (inChild.type) {
                 case DirectoryHandler::IsUnknown:
                    // Ignore unknown
                    break;

                 case DirectoryHandler::IsFile:
                    // File: just overwrite
                    copyChild(out, in, inChild);
                    break;

                 case DirectoryHandler::IsDirectory: {
                    // Recursively erase directory
                    std::auto_ptr<DirectoryHandler> inDir(in.getDirectory(inChild));
                    std::auto_ptr<DirectoryHandler> outDir(out.getDirectory(outChild));
                    synchronizeDirectories(*outDir, *inDir);
                    break;
                 }
                }
            }
            ++inIndex, ++outIndex;
        }
    }
    while (inIndex < inChildren.size()) {
        // copy missing child
        copyChild(out, in, inChildren[inIndex]);
        ++inIndex;
    }
    while (outIndex < outChildren.size()) {
        // remove superfluous child
        removeChild(out, outChildren[outIndex]);
        ++outIndex;
    }
}
