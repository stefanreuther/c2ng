/**
  *  \file ui/reshack/editor.hpp
  *  \brief Interface ui::reshack::Editor
  */
#ifndef C2NG_UI_RESHACK_EDITOR_HPP
#define C2NG_UI_RESHACK_EDITOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace ui { namespace reshack {

    class Session;

    /** Editor base class. */
    class Editor : public afl::base::Deletable {
        // ex RHEditor
     public:
        /** Get name of this editor. Displayed in the editor list. */
        virtual String_t getName(afl::string::Translator& tx) = 0;

        /** Edit this item ("Edit" command).
            @param session Session (contains required environment objects) */
        virtual void edit(Session& session) = 0;

        /** Save this item ("Save" command).
            @param session Session (contains required environment objects) */
        virtual void save(Session& session) = 0;

        /** Check for possibly-unsaved changes. */
        virtual bool hasUnsavedChanges() = 0;
    };

} }

#endif
