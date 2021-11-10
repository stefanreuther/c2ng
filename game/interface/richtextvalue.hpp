/**
  *  \file game/interface/richtextvalue.hpp
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "interpreter/basevalue.hpp"
#include "util/rich/text.hpp"

namespace game { namespace interface {

    class RichTextValue : public interpreter::BaseValue {
     public:
        typedef afl::base::Ptr<util::rich::Text> Ptr_t;
        typedef afl::base::Ref<util::rich::Text> Ref_t;

        RichTextValue(Ref_t value);
        ~RichTextValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // Value:
        virtual RichTextValue* clone() const;

        Ref_t get() const;

     private:
        Ref_t m_value;
    };

} }

#endif
