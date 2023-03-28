/**
  *  \file game/interface/richtextvalue.hpp
  *  \brief Class game::interface::RichTextValue
  */
#ifndef C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP
#define C2NG_GAME_INTERFACE_RICHTEXTVALUE_HPP

#include "afl/base/ptr.hpp"
#include "afl/base/ref.hpp"
#include "interpreter/basevalue.hpp"
#include "util/rich/text.hpp"

namespace game { namespace interface {

    /** Rich-text value.
        Stores a read-only, shared copy of a util::rich::Text object.

        (This does not use GenericValue to support a custom toString().) */
    class RichTextValue : public interpreter::BaseValue {
     public:
        /** Shortcut for nullable pointer to rich-text object. */
        typedef afl::base::Ptr<const util::rich::Text> Ptr_t;

        /** Shortcut for non-nullable pointer to rich-text object. */
        typedef afl::base::Ref<const util::rich::Text> Ref_t;

        /** Constructor.
            @param value Value */
        explicit RichTextValue(Ref_t value);

        /** Destructor. */
        ~RichTextValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const;

        // Value:
        virtual RichTextValue* clone() const;

        /** Get contained pointer.
            @return contained rich-text object */
        const Ref_t& get() const;

     private:
        Ref_t m_value;
    };

} }

#endif
