/**
  *  \file server/host/file/toolitem.hpp
  *  \brief Class server::host::file::ToolItem
  */
#ifndef C2NG_SERVER_HOST_FILE_TOOLITEM_HPP
#define C2NG_SERVER_HOST_FILE_TOOLITEM_HPP

#include "server/host/file/item.hpp"
#include "server/host/root.hpp"
#include "server/host/session.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace host { namespace file {

    /** Directory for a tool.
        Contains a listable set of files. */
    class ToolItem : public Item {
     public:
        /** Constructor.
            @param session      Session (for access checking)
            @param filer        Filer instance
            @param name         Tool name (also directory name in c2host's virtual file tree)
            @param pathName     Path name in filer
            @param title        Human-readable tool name
            @param restriction  If specified, comma-separated list of files to list; if not given, all files from the filer are listed */
        ToolItem(const Session& session, afl::net::CommandHandler& filer, String_t name, String_t pathName, String_t title, afl::base::Optional<String_t> restriction);

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
        const String_t m_pathName;
        const String_t m_title;
        const afl::base::Optional<String_t> m_restriction;
    };

} } }

#endif
