/**
  *  \file server/file/ca/root.hpp
  */
#ifndef C2NG_SERVER_FILE_CA_ROOT_HPP
#define C2NG_SERVER_FILE_CA_ROOT_HPP

#include <memory>
#include "afl/base/deletable.hpp"
#include "server/file/directoryhandler.hpp"

namespace server { namespace file { namespace ca {

    class ObjectStore;

    class Root : public afl::base::Deletable {
     public:
        explicit Root(server::file::DirectoryHandler& root);
        ~Root();

        server::file::DirectoryHandler* createRootHandler();

     private:
        class RootUpdater;

        void init();

        server::file::DirectoryHandler& m_root;

        std::auto_ptr<server::file::DirectoryHandler> m_refs;
        std::auto_ptr<server::file::DirectoryHandler> m_refsHeads;
        std::auto_ptr<server::file::DirectoryHandler> m_objects;

        std::auto_ptr<ObjectStore> m_store;
    };

} } }

#endif
