/**
  *  \file game/interface/richtextvalue.hpp
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP

#include "afl/base/ptr.hpp"
#include "util/rich/text.hpp"
#include "interpreter/basevalue.hpp"

namespace game { namespace interface {

    class RichTextValue : public interpreter::BaseValue {
     public:
        typedef afl::base::Ptr<util::rich::Text> Ptr_t;

        RichTextValue(Ptr_t value);
        ~RichTextValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext* ctx) const;

        // Value:
        virtual RichTextValue* clone() const;

        Ptr_t get();

     private:
        Ptr_t m_value;
    };

} }

#endif
