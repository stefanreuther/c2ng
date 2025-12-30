/**
  *  \file ui/reshack/session.hpp
  *  \brief Class ui::reshack::Session
  */
#ifndef C2NG_UI_RESHACK_SESSION_HPP
#define C2NG_UI_RESHACK_SESSION_HPP

#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "gfx/fontlist.hpp"
#include "ui/res/manager.hpp"
#include "ui/reshack/clipboard.hpp"
#include "ui/root.hpp"
#include "util/characternamelist.hpp"

namespace ui { namespace reshack {

    /** Central object aggregation for reshack. */
    class Session {
     public:
        /** Constructor.
            @param root      UI root
            @param tx        Translator
            @param fs        FileSystem
            @param mgr       UI resource manager (for loading images)
            @param fontList  Font List (for accessing list of system fonts) */
        Session(Root& root, afl::string::Translator& tx, afl::io::FileSystem& fs, ui::res::Manager& mgr, const gfx::FontList& fontList)
            : m_root(root), m_translator(tx), m_fileSystem(fs), m_manager(mgr), m_fontList(fontList),
              m_clipboard(), m_characterNames(), m_previewText()
            { }

        /** Access UI root.
            @return root as given to constructor. */
        Root& root()
            { return m_root; }

        /** Access translator.
            @return translator as given to constructor. */
        afl::string::Translator& translator()
            { return m_translator; }

        /** Access file system.
            @return file system as given to constructor. */
        afl::io::FileSystem& fileSystem()
            { return m_fileSystem; }

        /** Access UI resource manager.
            @return UI resource manager as given to constructor. */
        ui::res::Manager& manager()
            { return m_manager; }

        /** Access font list.
            @return font list as given to constructor. */
        const gfx::FontList& fontList()
            { return m_fontList; }

        /** Access clipboard.
            @return embedded clipboard instance */
        Clipboard& clipboard()
            { return m_clipboard; }

        /** Access character name list.
            @return embedded character name list */
        util::CharacterNameList& characterNames()
            { return m_characterNames; }

        /** Set preview text.
            Sets the value returned by getPreviewText().
            This stores status for the "preview" dialog.
            @param text Text */
        void setPreviewText(const String_t& text)
            { m_previewText = text; }

        /** Get preview text.
            @return value set by setPreviewText(). */
        const String_t& getPreviewText() const
            { return m_previewText; }

     private:
        Root& m_root;
        afl::string::Translator& m_translator;
        afl::io::FileSystem& m_fileSystem;
        ui::res::Manager& m_manager;
        const gfx::FontList& m_fontList;
        Clipboard m_clipboard;
        util::CharacterNameList m_characterNames;
        String_t m_previewText;
    };

} }

#endif
