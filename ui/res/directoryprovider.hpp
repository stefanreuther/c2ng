/**
  *  \file ui/res/directoryprovider.hpp
  */
#ifndef C2NG_UI_RES_DIRECTORYPROVIDER_HPP
#define C2NG_UI_RES_DIRECTORYPROVIDER_HPP

#include <map>
#include "ui/res/provider.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/directory.hpp"
#include "afl/string/string.hpp"

namespace ui { namespace res {

// /** Resources from a directory (dir:NAME). This provides resources
//     from files in a directory. File names are either synthesized from
//     the ResId, or taken from an alias table. The alias table can refer
//     to file names, and build synthetic resources. */
    class DirectoryProvider : public Provider {
     public:
        DirectoryProvider(afl::base::Ref<afl::io::Directory> dir);

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        afl::base::Ref<afl::io::Directory> m_directory;

        typedef std::map<String_t, String_t> AliasMap_t;
        AliasMap_t m_aliasMap;

        void loadAliases();
    };

} }

#endif
