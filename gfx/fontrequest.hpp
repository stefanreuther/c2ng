/**
  *  \file gfx/fontrequest.hpp
  *  \brief Class gfx::FontRequest
  */
#ifndef C2NG_GFX_FONTREQUEST_HPP
#define C2NG_GFX_FONTREQUEST_HPP

#include "afl/base/types.hpp"
#include "afl/base/inlineoptional.hpp"

namespace gfx {

    /** Font request.
        Symbolically describes a font for lookup/match in a list.
        A font is identified by four values:
        - size. 0=default size, positive values make larger, negative values make smaller.
        - weight. 0=regular, positive values make bolder, negative values make thinner.
        - slant. 0=upright, positive values for forward-italic.
        - style. Application-specific value, to distinguish e.g. serif, sans, mono. */
    class FontRequest {
     public:
        /** Parameter for a font request, raw. */
        typedef int16_t RawValue_t;

        /** Parameter for a font request.
            Can be a number or "don't care/unknown". */
        typedef afl::base::InlineOptional<RawValue_t,-32767-1> Value_t;

        /** Default constructor.
            Creates a font request with all parameters known-zero (=default font). */
        FontRequest();
        // FontRequest(afl::base::NothingType);

        /** Add size.
            Increases the size; if size is currently unknown, sets it.
            \param n size increase (positive: larger)
            \return *this */
        FontRequest& addSize(int n);

        /** Add weight.
            Increases the weight; if size is currently unknown, sets it.
            \param n weight increase (positive: bolder)
            \return *this */
        FontRequest& addWeight(int n);

        /** Set size.
            \param n Size
            \return *this */
        FontRequest& setSize(Value_t n);

        /** Set weight.
            \param n Weight
            \return *this */
        FontRequest& setWeight(Value_t n);

        /** Set slant.
            \param n Slant
            \return *this */
        FontRequest& setSlant(Value_t n);

        /** Set style.
            \param n Style
            \return *this */
        FontRequest& setStyle(Value_t n);

        /** Get size.
            \return size */
        Value_t getSize() const;

        /** Get weight.
            \return weight */
        Value_t getWeight() const;

        /** Get slant.
            \return slant */
        Value_t getSlant() const;

        /** Get style.
            \return style */
        Value_t getStyle() const;

        /** Match another FontRequest.
            Two font requests match if their parameters are identical, or set to "don't care".
            \param provided Other FontRequest
            \return true on match */
        bool match(const FontRequest& provided);

     private:
        Value_t m_size;
        Value_t m_weight;
        Value_t m_slant;
        Value_t m_style;
    };

}

#endif
