/**
  *  \file gfx/fontrequest.hpp
  */
#ifndef C2NG_GFX_FONTREQUEST_HPP
#define C2NG_GFX_FONTREQUEST_HPP

#include "afl/base/types.hpp"
#include "afl/base/inlineoptional.hpp"

namespace gfx {

    class FontRequest {
     public:
        typedef afl::base::InlineOptional<int16_t,-32767-1> Value_t;

        FontRequest();
        // FontRequest(afl::base::NothingType);

        FontRequest& addSize(int n);
        FontRequest& addWeight(int n);

        FontRequest& setSize(Value_t n);
        FontRequest& setWeight(Value_t n);
        FontRequest& setSlant(Value_t n);
        FontRequest& setStyle(Value_t n);

        Value_t getSize() const;
        Value_t getWeight() const;
        Value_t getSlant() const;
        Value_t getStyle() const;

        bool match(const FontRequest& provided);

     private:
        Value_t m_size;
        Value_t m_weight;
        Value_t m_slant;
        Value_t m_style;
    };

}

#endif
