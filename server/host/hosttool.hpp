/**
  *  \file server/host/hosttool.hpp
  *  \brief Class server::host::HostTool
  */
#ifndef C2NG_SERVER_HOST_HOSTTOOL_HPP
#define C2NG_SERVER_HOST_HOSTTOOL_HPP

#include "server/host/root.hpp"
#include "server/interface/hosttool.hpp"

namespace server { namespace host {

    class Session;
    class Root;

    /** Implementation of HostTool interface.
        There's a parallel set of HOST, MASTER, TOOL and SHIPLIST commands to manage game components.
        They are implemented identically, their domain is handed in as a database subtree. */
    class HostTool : public server::interface::HostTool {
     public:
        /** Constructor.
            \param session Session
            \param root    Service root
            \param tree    Database tree to use */
        HostTool(const Session& session, Root& root, Root::ToolTree tree);

        // HostTool:
        virtual void add(String_t id, String_t path, String_t program, String_t kind);
        virtual void set(String_t id, String_t key, String_t value);
        virtual String_t get(String_t id, String_t key);
        virtual bool remove(String_t id);
        virtual void getAll(std::vector<Info>& result);
        virtual void copy(String_t sourceId, String_t destinationId);
        virtual void setDefault(String_t id);
        virtual int32_t getDifficulty(String_t id);
        virtual void clearDifficulty(String_t id);
        virtual int32_t setDifficulty(String_t id, afl::base::Optional<int32_t> value, bool use);

     private:
        const Session& m_session;
        Root& m_root;
        Root::ToolTree m_tree;
    };

} }

#endif
