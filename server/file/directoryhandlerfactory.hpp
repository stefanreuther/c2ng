/**
  *  \file server/file/directoryhandlerfactory.hpp
  *  \brief Class server::file::DirectoryHandlerFactory
  */
#ifndef C2NG_SERVER_FILE_DIRECTORYHANDLERFACTORY_HPP
#define C2NG_SERVER_FILE_DIRECTORYHANDLERFACTORY_HPP

#include <map>
#include "afl/io/filesystem.hpp"
#include "afl/base/deleter.hpp"
#include "afl/net/networkstack.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace file {

    class DirectoryHandler;

    /** Factory for DirectoryHandler instances.
        This is mainly used to create the back-ends of a c2file storage.

        DirectoryHandlerFactory controls the lifetime of the DirectoryHandler objects it creates,
        and optimizes by re-using objects if possible.
        This allows downstream code to again optimize when seeing related directories
        (e.g. by doing server-side copies). */
    class DirectoryHandlerFactory {
     public:
        /** Constructor.
            \param fs File System
            \param net Network Stack */
        explicit DirectoryHandlerFactory(afl::io::FileSystem& fs, afl::net::NetworkStack& net);

        /** Create a DirectoryHandler.
            \param str Descriptor
            \return DirectoryHandler instance. This object lives as long as the DirectoryHandlerFactory. */
        DirectoryHandler& createDirectoryHandler(const String_t& str);

        /** Build a path name.
            Assuming \c backendPath is a path describing a file space, and \c child names a directory in it,
            returns the path describing that directory.
            In terms of functions,
            <code>createDirectoryHandler(backendPath).getDirectory(child) == createDirectoryHandler(makePathName(backendPath, child))</code>

            \param backendPath Path to file space
            \param child Directory name
            \return combined name */
        static String_t makePathName(const String_t& backendPath, const String_t& child);

     private:
        typedef std::map<String_t, DirectoryHandler*> Cache_t;
        typedef std::map<String_t, afl::net::CommandHandler*> ClientCache_t;

        afl::base::Deleter m_deleter;
        Cache_t m_cache;
        ClientCache_t m_clientCache;
        afl::io::FileSystem& m_fs;
        afl::net::NetworkStack& m_networkStack;
    };

} }

#endif
