/**
  *  \file gfx/pixmap.hpp
  */
#ifndef C2NG_GFX_PIXMAP_HPP
#define C2NG_GFX_PIXMAP_HPP

#include <algorithm>
#include "afl/base/growablememory.hpp"
#include "gfx/point.hpp"

namespace gfx {

    template<typename T>
    class Pixmap {
     public:
        Pixmap(int w, int h);

        ~Pixmap();

        afl::base::Memory<T> pixels();

        afl::base::Memory<const T> pixels() const;

        afl::base::Memory<T> row(int y);

        afl::base::Memory<const T> row(int y) const;

        void flipHorizontal();

        void flipVertical();

        Point getSize() const;

        int getWidth() const;

        int getHeight() const;

     private:
        int m_width;
        int m_height;
        afl::base::GrowableMemory<T> m_pixels;
    };

}

template<typename T>
gfx::Pixmap<T>::Pixmap(int w, int h)
    : m_width(std::max(0, w)),
      m_height(std::max(0, h)),
      m_pixels()
{
    m_pixels.ensureSize(m_width * m_height);
}

template<typename T>
gfx::Pixmap<T>::~Pixmap()
{ }

template<typename T>
inline afl::base::Memory<T>
gfx::Pixmap<T>::pixels()
{
    return m_pixels;
}

template<typename T>
inline afl::base::Memory<const T>
gfx::Pixmap<T>::pixels() const
{
    return m_pixels;
}

template<typename T>
afl::base::Memory<T>
gfx::Pixmap<T>::row(int y)
{
    return (y >= 0 && y < m_height)
        ? m_pixels.subrange(y * m_width, m_width)
        : afl::base::Memory<T>();
}

template<typename T>
afl::base::Memory<const T>
gfx::Pixmap<T>::row(int y) const
{
    return (y >= 0 && y < m_height)
        ? m_pixels.subrange(y * m_width, m_width)
        : afl::base::Memory<const T>();
}

template<typename T>
void
gfx::Pixmap<T>::flipHorizontal()
{
    for (int y = 0; y < m_height; ++y) {
        afl::base::Memory<T> r = row(y);
        int x1 = 0;
        int x2 = m_width-1;
        while (x1 < x2) {
            std::swap(*r.at(x1), *r.at(x2));
            ++x1;
            --x2;
        }
    }
}

template<typename T>
void
gfx::Pixmap<T>::flipVertical()
{
    afl::base::GrowableMemory<T> tmp;
    tmp.ensureSize(m_width);
    int y1 = 0;
    int y2 = m_height-1;
    while (y1 < y2) {
        tmp.copyFrom(row(y1));
        row(y1).copyFrom(row(y2));
        row(y2).copyFrom(tmp);
        ++y1;
        --y2;
    }
}

template<typename T>
inline gfx::Point
gfx::Pixmap<T>::getSize() const
{
    return Point(m_width, m_height);
}

template<typename T>
inline int
gfx::Pixmap<T>::getWidth() const
{
    return m_width;
}

template<typename T>
inline int
gfx::Pixmap<T>::getHeight() const
{
    return m_height;
}

#endif
