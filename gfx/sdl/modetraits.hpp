/**
  *  \file gfx/sdl/modetraits.hpp
  *
  *  Generalized access to SDL_Surface pixel data. Do not ever include
  *  this from a user .hpp file.
  */
#ifndef C2NG_GFX_SDL_MODETRAITS_HPP
#define C2NG_GFX_SDL_MODETRAITS_HPP

#include <SDL_video.h>
#include "afl/base/types.hpp"
#include "gfx/types.hpp"

namespace gfx { namespace sdl {

    /** \defgroup gfx_mode_traits Graphics Mode Traits
        These define access to surfaces of various depths.
        Each type supports the following expressions:
    
        - T::pix_type      = type which can hold a pixel
        - T::peek(ptr)     = read video memory at specified address
        - T::poke(ptr,val) = modify video memory at specified address
        - T::add(ptr,diff) = advance ptr by diff pixels
        - T::mix(fmt,val,val,alpha) = alpha mixing */
    //@{
    /// Traits for 8-bit pixel-mapped surface.
    struct ModeTraits8 {
        typedef uint8_t Pixel_t;
        typedef uint8_t Data_t;
        
        Data_t* get(int x, int y) const                               { return add((Data_t*)sfc->pixels, x, y); }
        static inline Pixel_t peek(Data_t* ptr)                       { return *ptr; }
        static inline void poke(Data_t* ptr, Pixel_t val)             { *ptr = val; }
        inline Pixel_t mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const;
        inline Data_t* add(Data_t* ptr, int dx, int dy) const         { return ptr + sfc->pitch*dy + dx; }

        ModeTraits8(SDL_Surface* sfc)
            : sfc(sfc)
            { }
        SDL_Surface* sfc;
    };

    /// Traits for 16-bit color-mapped surface (realcolor).
    struct ModeTraits16 {
        typedef uint16_t Pixel_t;
        typedef uint8_t Data_t;

        Data_t* get(int x, int y) const                               { return add((Data_t*)sfc->pixels, x, y); }
        static inline Pixel_t peek(Data_t* ptr)                       { return *(Pixel_t*)ptr; }
        static inline void poke(Data_t* ptr, Pixel_t val)             { *(Pixel_t*)ptr = val; }
        inline Pixel_t mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const;
        inline Data_t* add(Data_t* ptr, int dx, int dy) const         { return ptr + sfc->pitch*dy + 2*dx; }

        ModeTraits16(SDL_Surface* sfc)
            : sfc(sfc)
            { }
        SDL_Surface* sfc;
    };

    /// Traits for 32-bit color-mapped surface (truecolor).
    struct ModeTraits32 {
        typedef uint32_t Pixel_t;
        typedef uint8_t Data_t;

        Data_t* get(int x, int y) const                              { return add((Data_t*)sfc->pixels, x, y); }
        static inline Pixel_t peek(Data_t* ptr)                      { return *(Pixel_t*)ptr; }
        static inline void poke(Data_t* ptr, Pixel_t val)            { *(Pixel_t*)ptr = val; }
        inline Pixel_t mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const;
        inline Data_t* add(Data_t* ptr, int dx, int dy) const        { return ptr + sfc->pitch*dy + 4*dx; }

        ModeTraits32(SDL_Surface* sfc)
            : sfc(sfc)
            { }
        SDL_Surface* sfc;
    };

    /// Traits for 24-bit color-mapped surface (truecolor).
    struct ModeTraits24 {
        typedef uint32_t Pixel_t;
        typedef uint8_t Data_t;

        Data_t* get(int x, int y) const
            {
                return add((Data_t*)sfc->pixels, x, y);
            }
        static inline Pixel_t peek(Data_t* ptr)
            {
                return ptr[0] + 256U*ptr[1] + 65536U*ptr[2];
            }
        static inline void poke(Data_t* ptr, Pixel_t val)
            {
                ptr[0] = val;
                ptr[1] = val >> 8;
                ptr[2] = val >> 16;
            }
        inline Pixel_t mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const
            {
                return ModeTraits32(sfc).mix(a, b, balpha);
            }
        inline Data_t* add(Data_t* ptr, int dx, int dy) const
            {
                return ptr + sfc->pitch*dy + 3*dx;
            }

        ModeTraits24(SDL_Surface* sfc)
            : sfc(sfc)
            { }
        SDL_Surface* sfc;
    };
    //@}

} }

gfx::sdl::ModeTraits8::Pixel_t
gfx::sdl::ModeTraits8::mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const
{
    /* FIXME: caching? */
    register SDL_Color* pal = sfc->format->palette->colors;
    return SDL_MapRGB(sfc->format,
                      mixColorComponent(pal[a].r, pal[b].r, balpha),
                      mixColorComponent(pal[a].g, pal[b].g, balpha),
                      mixColorComponent(pal[a].b, pal[b].b, balpha));
}

gfx::sdl::ModeTraits16::Pixel_t
gfx::sdl::ModeTraits16::mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const
{
    SDL_PixelFormat* fmt = sfc->format;
    Pixel_t re = mixColorComponent(a & fmt->Rmask, b & fmt->Rmask, balpha) & fmt->Rmask;
    Pixel_t gr = mixColorComponent(a & fmt->Gmask, b & fmt->Gmask, balpha) & fmt->Gmask;
    Pixel_t bl = mixColorComponent(a & fmt->Bmask, b & fmt->Bmask, balpha) & fmt->Bmask;
    return re | gr | bl;
}

gfx::sdl::ModeTraits32::Pixel_t
gfx::sdl::ModeTraits32::mix(Pixel_t a, Pixel_t b, Alpha_t balpha) const
{
    /* FIXME: Alpha channel (Amask) ? */
    /* FIXME2: can this overflow (if Rmask=0xFF000000)? */
    SDL_PixelFormat* fmt = sfc->format;
    Pixel_t re = mixColorComponent(a & fmt->Rmask, b & fmt->Rmask, balpha) & fmt->Rmask;
    Pixel_t gr = mixColorComponent(a & fmt->Gmask, b & fmt->Gmask, balpha) & fmt->Gmask;
    Pixel_t bl = mixColorComponent(a & fmt->Bmask, b & fmt->Bmask, balpha) & fmt->Bmask;
    return re | gr | bl;
}

/** Pixel Format Switch.
    \param function name of a function defined as
                    `template<class T> void func(args)'
    \param args     arguments to use for call

    Calls the correct function with the appropriate mode traits filled
    in. The surface being examined must be in scope under the name
    `sfc'. */
#define GFX_MODE_SWITCH(sfc, call)                             \
    do switch(sfc->format->BytesPerPixel) {                    \
     case 1:                                                   \
        Primitives<gfx::sdl::ModeTraits8>(sfc).call;           \
        break;                                                 \
     case 2: {                                                 \
        Primitives<gfx::sdl::ModeTraits16>(sfc).call;          \
        break;                                                 \
     }                                                         \
     case 3: {                                                 \
        Primitives<gfx::sdl::ModeTraits24>(sfc).call;          \
        break;                                                 \
     }                                                         \
     case 4: {                                                 \
        Primitives<gfx::sdl::ModeTraits32>(sfc).call;          \
        break;                                                 \
     }                                                         \
    } while(0)

#endif

