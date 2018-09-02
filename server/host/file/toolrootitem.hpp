/**
  *  \file server/host/file/toolrootitem.hpp
  */
#ifndef C2NG_SERVER_HOST_FILE_TOOLROOTITEM_HPP
#define C2NG_SERVER_HOST_FILE_TOOLROOTITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"

namespace server { namespace host { namespace file {

    class ToolRootItem : public Item {
     public:
        ToolRootItem(Session& session, afl::net::CommandHandler& filer, String_t name, const Root::ToolTree& tree, bool restricted);

        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        Session& m_session;
        afl::net::CommandHandler& m_filer;
        String_t m_name;
        Root::ToolTree m_tree;
        bool m_restricted;
    };

} } }

#endif
