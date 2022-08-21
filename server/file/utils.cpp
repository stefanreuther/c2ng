/**
  *  \file server/file/utils.cpp
  *  \brief server::file Utilities
  */

#include <memory>
#include "server/file/utils.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/archive/tarreader.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/inflatetransform.hpp"
#include "afl/io/transformreaderstream.hpp"
#include "server/errors.hpp"
#include "server/file/directoryhandler.hpp"

using server::file::DirectoryHandler;
using server::file::ReadOnlyDirectoryHandler;

namespace {
    /** Comparator for DirectoryHandler::Info; sort by name. */
    struct CompareListItems {
        bool operator()(const DirectoryHandler::Info& a, const DirectoryHandler::Info& b)
            { return a.name < b.name; }
    };

    /** Split extension off a file name.
        \param fullName [in] Full file name (without directory etc.)
        \param ext      [in] Expected extension
        \param baseName [out] Basename
        \retval true Extension matched, baseName has been set
        \retval false Extension did not match */
    bool splitExtension(const String_t& fullName, const char* ext, String_t& baseName)
    {
        // FIXME: duplicated in server/host/exporter.cpp
        size_t n = std::strlen(ext);
        if (fullName.size() > n) {
            size_t splitPoint = fullName.size() - n;
            if (fullName.compare(splitPoint, n, ext, n) == 0) {
                baseName.assign(fullName, 0, splitPoint);
                return true;
            }
        }
        return false;
    }

    /** Copy a file.
        \param out Output directory
        \param in Input directory
        \param inChild Input file info */
    void copyFile(DirectoryHandler& out, ReadOnlyDirectoryHandler& in, const DirectoryHandler::Info& inChild)
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
    void copyChild(DirectoryHandler& out, ReadOnlyDirectoryHandler& in, const DirectoryHandler::Info& inChild)
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
            std::auto_ptr<ReadOnlyDirectoryHandler> inHandler(in.getDirectory(inChild));
            copyDirectory(*outHandler, *inHandler, server::file::CopyFlags_t(server::file::CopyRecursively));
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

    void copyTarball(DirectoryHandler& out, ReadOnlyDirectoryHandler& in, const DirectoryHandler::Info& inChild, const String_t& outName)
    {
        // Get input file content
        afl::base::Ref<afl::io::FileMapping> inMapping = in.getFile(inChild);
        afl::io::ConstMemoryStream inStream(inMapping->get());

        // Set up for reading a tarball
        afl::io::InflateTransform tx(afl::io::InflateTransform::Gzip);
        afl::base::Ref<afl::io::Stream> reader(*new afl::io::TransformReaderStream(inStream, tx));
        afl::base::Ref<afl::io::Directory> dir(afl::io::archive::TarReader::open(reader, 0));

        // Create directory
        std::auto_ptr<server::file::DirectoryHandler> target(out.getDirectory(out.createDirectory(outName)));

        // Copy content, one-by-one, but in lock-step order.
        // This is required to allow reading a .tar.gz (which does not support random access) on-the-fly.
        afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > tarballContent(dir->getDirectoryEntries());
        afl::base::Ptr<afl::io::DirectoryEntry> tarballEle;
        while (tarballContent->getNextElement(tarballEle) && tarballEle.get() != 0) {
            if (tarballEle->getFileType() == afl::io::DirectoryEntry::tFile) {
                // Use a virtual file mapping. This gives maximum possiblities to avoid copying.
                afl::base::Ref<afl::io::FileMapping> content = tarballEle->openFile(afl::io::FileSystem::OpenRead)->createVirtualMapping();
                target->createFile(tarballEle->getTitle(), content->get());
            }
        }
    }
}


// List a directory.
void
server::file::listDirectory(InfoVector_t& out, ReadOnlyDirectoryHandler& dir)
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
server::file::copyDirectory(DirectoryHandler& out, ReadOnlyDirectoryHandler& in, CopyFlags_t flags)
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

         case DirectoryHandler::IsFile: {
            // Copy file
            String_t elementBaseName;
            if (flags.contains(CopyExpandTarballs)
                && (splitExtension(ch.name, ".tar.gz", elementBaseName)
                    || splitExtension(ch.name, ".tgz", elementBaseName)))
            {
                // Compressed file
                copyTarball(out, in, ch, elementBaseName);
            } else {
                // Normal file
                copyFile(out, in, ch);
            }
            break;
         }

         case DirectoryHandler::IsDirectory:
            // Recursively copy directory if desired
            if (flags.contains(CopyRecursively)) {
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
                std::auto_ptr<ReadOnlyDirectoryHandler> inHandler(in.getDirectory(ch));
                copyDirectory(*outHandler, *inHandler, flags);
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
server::file::synchronizeDirectories(DirectoryHandler& out, ReadOnlyDirectoryHandler& in)
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
                    std::auto_ptr<ReadOnlyDirectoryHandler> inDir(in.getDirectory(inChild));
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
