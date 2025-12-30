/**
  *  \file ui/reshack/pictureeditor.hpp
  *  \brief Class ui::reshack::PictureEditor
  */
#ifndef C2NG_UI_RESHACK_PICTUREEDITOR_HPP
#define C2NG_UI_RESHACK_PICTUREEDITOR_HPP

#include "gfx/palettizedpixmap.hpp"
#include "ui/reshack/editor.hpp"

namespace ui { namespace reshack {

    /** Picture editor.
        Implements the Editor interface for an image (PalettizedPixmap). */
    class PictureEditor : public Editor {
     public:
        /** Constructor.
            @param pix The picture
            @param fileName File name; can be empty */
        PictureEditor(afl::base::Ref<gfx::PalettizedPixmap> pix, String_t fileName);

        // Editor:
        virtual String_t getName(afl::string::Translator& tx);
        virtual void edit(Session& session);
        virtual void save(Session& session);
        virtual bool hasUnsavedChanges();

     private:
        class SaveDialog;
        class Dialog;

        afl::base::Ptr<gfx::PalettizedPixmap> m_pixmap;
        String_t m_fileName;
        bool m_dirty;

        void saveImage(Session& session, int fileFormat, String_t fileName);
    };

} }

#endif
