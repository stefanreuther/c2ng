/**
  *  \file server/play/fs/directory.cpp
  *  \brief Class server::play::fs::Directory
  */

#include "server/play/fs/directory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/posixfilenames.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"
#include "util/serverdirectory.hpp"

using afl::base::Ref;
using afl::string::PosixFileNames;
using server::interface::FileBase;
using server::interface::FileBaseClient;

namespace {
    /* Externally provided file names must start with a slash,
       but file names used in the server communication must not start with a slash. */
    String_t trimSlash(String_t pathName)
    {
        if (pathName.empty() || pathName[0] != '/') {
            throw afl::except::FileProblemException(pathName, "<invalid name>");
        }
        return pathName.substr(1);
    }
}

class server::play::fs::Directory::Transport : public util::ServerDirectory::Transport {
 public:
    Transport(const afl::base::Ref<Session>& session, const String_t& dirName)
        : m_session(session), m_dirName(dirName)
        { }

    virtual void getFile(String_t name, afl::base::GrowableBytes_t& data)
        {
            data.append(afl::string::toBytes(FileBaseClient(m_session->fileClient()).getFile(makePathName(name))));
        }

    virtual void putFile(String_t name, afl::base::ConstBytes_t data)
        {
            FileBaseClient(m_session->fileClient()).putFile(makePathName(name), afl::string::fromBytes(data));
        }

    virtual void eraseFile(String_t name)
        {
            FileBaseClient(m_session->fileClient()).removeFile(makePathName(name));
        }

    virtual void getContent(std::vector<util::ServerDirectory::FileInfo>& result)
        {
            FileBase::ContentInfoMap_t content;
            FileBaseClient(m_session->fileClient()).getDirectoryContent(trimSlash(m_dirName), content);
            for (FileBase::ContentInfoMap_t::const_iterator it = content.begin(); it != content.end(); ++it) {
                result.push_back(util::ServerDirectory::FileInfo(it->first, it->second->size.orElse(0), it->second->type == FileBase::IsFile));
            }
        }

    virtual bool isValidFileName(String_t name) const
        {
            return name.find_first_of("/:") == String_t::npos;
        }

    virtual bool isWritable() const
        {
            return true;
        }

 private:
    afl::base::Ref<Session> m_session;
    String_t m_dirName;

    String_t makePathName(const String_t& fileName) const
        {
            return trimSlash(PosixFileNames().makePathName(m_dirName, fileName));
        }
};

afl::base::Ref<afl::io::Directory>
server::play::fs::Directory::create(afl::base::Ref<Session> session, String_t dirName)
{
    return util::ServerDirectory::create(*new Transport(session, dirName), dirName, 0);
}
