/**
  *  \file server/host/file/toolrootitem.hpp
  *  \brief Class server::host::file::ToolRootItem
  */
#ifndef C2NG_SERVER_HOST_FILE_TOOLROOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_TOOLROOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    /** Top-level directory containing tools.
        Children are directories for the individual tools of the given type.
        Children can be requested by name, but not listed. */
    class ToolRootItem : public Item {
     public:
        /** Constructor.
            @param session    Session
            @param filer      Filer instance
            @param name       Name of this item (file name component)
            @param tree       Databse subtree
            @param restricted If true, report only files declared in the tool's "files" attribute (use for tools/add-ons).
                              If false, report all files in the filer for this tool (use for shiplists) */
        ToolRootItem(const Session& session, afl::net::CommandHandler& filer, String_t name, const Root::ToolTree& tree, bool restricted);

        // Item:
        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        const Session& m_session;
        afl::net::CommandHandler& m_filer;
        const String_t m_name;
        Root::ToolTree m_tree;
        const bool m_restricted;
    };

} } }

#endif
