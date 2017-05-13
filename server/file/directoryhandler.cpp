/**
  *  \file server/file/directoryhandler.cpp
  *  \brief Interface server::file::DirectoryHandler
  */

#include "server/file/directoryhandler.hpp"

bool
server::file::DirectoryHandler::findItem(String_t name, Info& info)
{
    class FindCallback : public Callback {
     public:
        FindCallback(const String_t& name, Info& info)
            : m_name(name),
              m_info(info),
              m_ok(false)
            { }
        virtual void addItem(const Info& info)
            {
                if (info.name == m_name) {
                    m_info = info;
                    m_ok = true;
                }
            }
        bool isOK() const
            { return m_ok; }
     private:
        const String_t& m_name;
        Info& m_info;
        bool m_ok;
    };
    FindCallback cb(name, info);
    readContent(cb);
    return cb.isOK();
}
