/**
  *  \file ui/widgets/decimalselector.hpp
  *  \brief Class ui::widgets::DecimalSelector
  */
#ifndef C2NG_UI_WIDGETS_DECIMALSELECTOR_HPP
#define C2NG_UI_WIDGETS_DECIMALSELECTOR_HPP

#include "afl/base/deletable.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "ui/root.hpp"
#include "ui/widgets/basedecimalselector.hpp"

namespace ui { namespace widgets {

    /** "Move-or-Type" number selector, regular type.
        This one looks similar to a regular input line. */
    class DecimalSelector : public BaseDecimalSelector {
     public:
        enum Flag {
            RightJustified,     // ex pds_RightJust
            ShowMaximum         // ex pds_ShowMax
        };
        typedef afl::bits::SmallSet<Flag> Flags_t;

        /** Constructor.
            \param root Root
            \param tx Translator
            \param value Storage for the value
            \param min Lower limit
            \param max Upper limit
            \param step Default step size */
        DecimalSelector(Root& root, afl::string::Translator& tx, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step);

        /** Destructor. */
        ~DecimalSelector();

        /** Set flag.
            Flags affect appearance or behaviour.
            \param flag Flag to modify
            \param enable set or clear the flag */
        void setFlag(Flag flag, bool enable);


        // BaseDecimalSelector:
        virtual void draw(gfx::Canvas& can);
        virtual ui::layout::Info getLayoutInfo() const;

     private:
        Root& m_root;
        afl::string::Translator& m_translator;
        Flags_t m_flags;
    };

} }

#endif
