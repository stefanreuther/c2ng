/**
  *  \file server/host/file/toolitem.hpp
  */
#ifndef C2NG_SERVER_HOST_FILE_TOOLITEM_HPP
#define C2NG_SERVER_HOST_FILE_TOOLITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace host { namespace file {

    class ToolItem : public Item {
     public:
        ToolItem(Session& session, afl::net::CommandHandler& filer, String_t name, String_t pathName, String_t title, afl::base::Optional<String_t> restriction);

        virtual String_t getName();
        virtual Info_t getInfo();
        virtual Item* find(const String_t& name);
        virtual void listContent(ItemVector_t& out);
        virtual String_t getContent();

     private:
        Session& m_session;
        afl::net::CommandHandler& m_filer;
        String_t m_name;
        String_t m_pathName;
        String_t m_title;
        afl::base::Optional<String_t> m_restriction;
    };

} } }

#endif
