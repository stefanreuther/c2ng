/**
  *  \file gfx/threed/colortransformation.cpp
  *  \brief Class gfx::threed::ColorTransformation
  */

#include "gfx/threed/colortransformation.hpp"

namespace {
    float colorComponentToFloat(uint8_t c)
    {
        return c * (1.0f / 255.0f);
    }

    uint8_t colorComponentFromFloat(float f)
    {
        f *= 255;
        f += 0.5f;

        // The '& 255' is required to handle NaN, which will otherwise produce INT_MIN despite reporting to be in range [0,255]...
        return f >= 255 ? 255 : f < 0 ? 0 : (uint8_t(f) & 255);
    }

    void multiplyRow(gfx::threed::ColorTransformation& out, const gfx::threed::ColorTransformation& a, const gfx::threed::ColorTransformation& b, size_t x)
    {
        // Cache only the current line of the second matrix
        float b0 = b(x), b1 = b(x+1), b2 = b(x+2), b3 = b(x+3);
        out(x)   = b0*a(0) + b1*a(4) + b2*a(8)  + b3*a(12);
        out(x+1) = b0*a(1) + b1*a(5) + b2*a(9)  + b3*a(13);
        out(x+2) = b0*a(2) + b1*a(6) + b2*a(10) + b3*a(14);
        out(x+3) = b0*a(3) + b1*a(7) + b2*a(11) + b3*a(15);
    }
}

/* Values from https://en.wikipedia.org/w/index.php?title=Grayscale&oldid=1162389246 */
const gfx::threed::Vec3f gfx::threed::ColorTransformation::GRAYSCALE_REC601(0.299f,  0.587f,  0.114f);
const gfx::threed::Vec3f gfx::threed::ColorTransformation::GRAYSCALE_BT709 (0.2126f, 0.7152f, 0.0722f);
const gfx::threed::Vec3f gfx::threed::ColorTransformation::GRAYSCALE_BT2100(0.2627f, 0.6780f, 0.0593f);

const gfx::threed::Vec3f gfx::threed::ColorTransformation::GRAYSCALE_SIMPLE(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);


gfx::threed::ColorTransformation::ColorTransformation(float a1, float a2, float a3, float a4,
                                                      float b1, float b2, float b3, float b4,
                                                      float c1, float c2, float c3, float c4,
                                                      float d1, float d2, float d3, float d4)
{
    // ex mMake (mClone)
    this->m_value[0]  = a1;
    this->m_value[1]  = a2;
    this->m_value[2]  = a3;
    this->m_value[3]  = a4;
    this->m_value[4]  = b1;
    this->m_value[5]  = b2;
    this->m_value[6]  = b3;
    this->m_value[7]  = b4;
    this->m_value[8]  = c1;
    this->m_value[9]  = c2;
    this->m_value[10] = c3;
    this->m_value[11] = c4;
    this->m_value[12] = d1;
    this->m_value[13] = d2;
    this->m_value[14] = d3;
    this->m_value[15] = d4;
}

gfx::threed::ColorTransformation
gfx::threed::ColorTransformation::identity()
{
    // ex mMakeIdentity
    return ColorTransformation(1, 0, 0, 0,
                               0, 1, 0, 0,
                               0, 0, 1, 0,
                               0, 0, 0, 1);
}


gfx::threed::ColorTransformation
gfx::threed::ColorTransformation::toGrayscale(ColorQuad_t color, Vec3f weights)
{
    Vec3f c = toFloat(color);
    return ColorTransformation(
        c(0) * weights(0), c(0) * weights(1), c(0) * weights(2), 0,
        c(1) * weights(0), c(1) * weights(1), c(1) * weights(2), 0,
        c(2) * weights(0), c(2) * weights(1), c(2) * weights(2), 0,
        0, 0, 0, 1);
}

gfx::threed::ColorTransformation
gfx::threed::ColorTransformation::toGrayscale(ColorQuad_t color)
{
    return toGrayscale(color, GRAYSCALE_REC601);
}

gfx::threed::ColorTransformation&
gfx::threed::ColorTransformation::scale(float factor)
{
    // Last row must always keep its value (0 0 0 1)
    for (size_t i = 0; i < 12; ++i) {
        (*this)(i) *= factor;
    }
    return *this;
}

gfx::threed::ColorTransformation&
gfx::threed::ColorTransformation::add(const Vec3f& vec)
{
    (*this)(3) += vec(0);
    (*this)(7) += vec(1);
    (*this)(11) += vec(2);
    return *this;
}

gfx::threed::ColorTransformation&
gfx::threed::ColorTransformation::add(ColorQuad_t color)
{
    return add(toFloat(color));
}

gfx::threed::ColorTransformation&
gfx::threed::ColorTransformation::operator*=(const ColorTransformation& other)
{
    // ex mMultiply
    ColorTransformation tmp = *this;
    multiplyRow(*this, tmp, other, 0);
    multiplyRow(*this, tmp, other, 4);
    multiplyRow(*this, tmp, other, 8);
    // multiplyRow(*this, tmp, other, 12);
    return *this;
}

gfx::threed::ColorTransformation
gfx::threed::ColorTransformation::operator*(const ColorTransformation& other) const
{
    // ex mMultiply
    ColorTransformation tmp = identity();
    multiplyRow(tmp, *this, other, 0);
    multiplyRow(tmp, *this, other, 4);
    multiplyRow(tmp, *this, other, 8);
    // multiplyRow(tmp, *this, other, 12);
    return tmp;
}


gfx::threed::ColorTransformation&
gfx::threed::ColorTransformation::operator+=(const ColorTransformation& other)
{
    for (size_t i = 0; i < 12; ++i) {
        (*this)(i) += other(i);
    }
    return *this;
}

gfx::threed::ColorTransformation
gfx::threed::ColorTransformation::operator+(const ColorTransformation& other) const
{
    ColorTransformation tmp(*this);
    tmp += other;
    return tmp;
}

gfx::threed::Vec3f
gfx::threed::ColorTransformation::transform(const Vec3f& vec) const
{
    // ex vTransform
    const float r = vec(0), g = vec(1), b = vec(2);
    return Vec3f((*this)(0) * r + (*this)(1) * g + (*this)(2)  * b + (*this)(3),
                 (*this)(4) * r + (*this)(5) * g + (*this)(6)  * b + (*this)(7),
                 (*this)(8) * r + (*this)(9) * g + (*this)(10) * b + (*this)(11));
}

gfx::ColorQuad_t
gfx::threed::ColorTransformation::transform(ColorQuad_t color) const
{
    return fromFloat(transform(toFloat(color)), ALPHA_FROM_COLORQUAD(color));
}


gfx::ColorQuad_t
gfx::threed::fromFloat(const Vec3f& vec, uint8_t alpha)
{
    return COLORQUAD_FROM_RGBA(colorComponentFromFloat(vec(0)),
                               colorComponentFromFloat(vec(1)),
                               colorComponentFromFloat(vec(2)),
                               alpha);
}

gfx::threed::Vec3f
gfx::threed::toFloat(ColorQuad_t color)
{
    return Vec3f(colorComponentToFloat(RED_FROM_COLORQUAD(color)),
                 colorComponentToFloat(GREEN_FROM_COLORQUAD(color)),
                 colorComponentToFloat(BLUE_FROM_COLORQUAD(color)));
}
