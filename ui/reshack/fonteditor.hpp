/**
  *  \file ui/reshack/fonteditor.hpp
  *  \brief Class ui::reshack::FontEditor
  */
#ifndef C2NG_UI_RESHACK_FONTEDITOR_HPP
#define C2NG_UI_RESHACK_FONTEDITOR_HPP

#include "gfx/bitmapfont.hpp"
#include "ui/reshack/editor.hpp"

namespace ui { namespace reshack {

    /** Font editor.
        Implements the Editor interface for a BitmapFont. */
    class FontEditor : public Editor {
     public:
        /** Constructor.
            @param font The font
            @param fileName File name; can be empty */
        FontEditor(afl::base::Ptr<gfx::BitmapFont> font, String_t fileName);

        // Editor
        virtual String_t getName(afl::string::Translator& tx);
        virtual void edit(Session& session);
        virtual void save(Session& session);
        virtual bool hasUnsavedChanges();

     private:
        class CharacterListDialog;
        class FontEditorWindow;
        class SaveDialog;

        afl::base::Ptr<gfx::BitmapFont> m_font;
        String_t m_fileName;
        bool m_dirty;
    };

} }

#endif
