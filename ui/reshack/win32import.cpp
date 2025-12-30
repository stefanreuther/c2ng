/**
  *  \file ui/reshack/win32import.cpp
  *  \brief Class ui::reshack::Win32Import
  */

#include "ui/reshack/win32import.hpp"

#ifdef TARGET_OS_WIN32
/*
 *  Windows - Real Implementation
 */
# include <windows.h>
# undef MessageBox

# include "afl/string/format.hpp"
# include "gfx/bitmapglyph.hpp"
# include "ui/dialogs/messagebox.hpp"
namespace {
    struct Objects {
        HFONT hFont;
        HDC   screenDC;
        HDC   virtDC;
    };

    bool createObjects(Objects& o, const LOGFONT& lf)
    {
        // Clear
        o.hFont    = 0;
        o.screenDC = 0;
        o.virtDC   = 0;

        // Create font
        o.hFont = CreateFontIndirect(&lf);
        if (!o.hFont) {
            return false;
        }

        // Create DCs
        o.screenDC = CreateDC("DISPLAY", 0, 0, 0);
        if (!o.screenDC) {
            return false;
        }
        o.virtDC = CreateCompatibleDC(o.screenDC);
        if (!o.virtDC) {
            return false;
        }

        // Enable anti-aliasing on memory DCs:
        // http://support.microsoft.com/kb/306198/en-us
        HDC hScreenIC = CreateIC("DISPLAY", NULL, NULL, NULL);
        HFONT hOldFont = (HFONT) SelectObject(hScreenIC, o.hFont);
        SelectObject(hScreenIC, hOldFont);
        DeleteDC(hScreenIC);

        return true;
    }

    void destroyObjects(Objects& o)
    {
        if (o.virtDC) {
            DeleteDC(o.virtDC);
        }
        if (o.screenDC) {
            DeleteDC(o.screenDC);
        }
        if (o.hFont) {
            DeleteObject(o.hFont);
        }
    }

}

bool
ui::reshack::Win32Import::isSupported()
{
    return true;
}

afl::base::Ptr<gfx::BitmapFont>
ui::reshack::Win32Import::importFont(Session& session)
{
    afl::string::Translator& tx = session.translator();
    Root& root = session.root();

    // Choose a font
    LOGFONT lf;
    CHOOSEFONT cf;
    ZeroMemory(&cf, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.hwndOwner   = 0;
    cf.hDC         = 0;
    cf.lpLogFont   = &lf;
    cf.Flags       = CF_SCREENFONTS;
    if (!ChooseFont(&cf)) {
        return 0;
    }

    // Raise quality
    lf.lfQuality = ANTIALIASED_QUALITY;

    // Create objects
    Objects o;
    if (!createObjects(o, lf)) {
        destroyObjects(o);
        ui::dialogs::MessageBox(tx("Unable to create system objects."), tx("Import Font"), root)
            .doOkDialog(tx);
        return 0;
    }
    SelectObject(o.virtDC, o.hFont);
    SelectObject(o.virtDC, GetStockObject(BLACK_BRUSH));
    SelectObject(o.virtDC, GetStockObject(BLACK_PEN));

    // Create font
    afl::base::Ptr<gfx::BitmapFont> newFont = new gfx::BitmapFont();

    // GetGlyphIndicesW exists on W2k, and is not in MinGW header files.
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-function-type"   // warning: cast between incompatible function types
# pragma GCC diagnostic ignored "-Wattributes"           // warning: dllimport attribute ignored
#endif
    typedef WINGDIAPI DWORD WINAPI (*tGetGlyphIndicesW)(IN HDC, IN LPCWSTR, IN int, OUT LPWORD, IN DWORD);
    tGetGlyphIndicesW pGetGlyphIndicesW = 0;
    HINSTANCE hGdi = LoadLibrary("gdi32.dll");
    if (hGdi != 0) {
        pGetGlyphIndicesW = reinterpret_cast<tGetGlyphIndicesW>(GetProcAddress(hGdi, "GetGlyphIndicesW"));
    }
#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif
#ifndef GGI_MARK_NONEXISTING_GLYPHS
    enum { GGI_MARK_NONEXISTING_GLYPHS = 1 };
#endif

    // Do it
    int errs = 0;
    int success = 0;
    for (afl::charset::Unichar_t i = 32; i < 0x3000; ++i) {
        wchar_t str = static_cast<wchar_t>(i);

        // Does this exist in the font? (and can we find out?)
        if (pGetGlyphIndicesW) {
            WORD gi;
            pGetGlyphIndicesW(o.virtDC, &str, 1, &gi, GGI_MARK_NONEXISTING_GLYPHS);
            if (gi == 0xFFFF) {
                continue;
            }
        }

        // Compute size
        RECT rect;
        rect.left = rect.top = 0;
        if (!DrawTextW(o.virtDC, &str, 1, &rect, DT_TOP | DT_LEFT | DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX)) {
            ++errs;
            continue;
        }
        if (rect.right == 0) {
            continue;
        }

        // Create bitmap to receive the character
        HBITMAP bmp = CreateBitmap(rect.right, rect.bottom, 1, 32, 0);
        if (!bmp) {
            ++errs;
            continue;
        }
        HBITMAP prevBMP = (HBITMAP) SelectObject(o.virtDC, bmp);
        FillRect(o.virtDC, &rect, (HBRUSH) GetStockObject(WHITE_BRUSH));

        // Draw character
        if (DrawTextW(o.virtDC, &str, 1, &rect, DT_TOP | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX)) {
            // Successfully drawn, so import the glyph
            std::auto_ptr<gfx::BitmapGlyph> glyph(new gfx::BitmapGlyph(static_cast<uint16_t>(rect.right), static_cast<uint16_t>(rect.bottom)));
            for (int y = 0; y < rect.bottom; ++y) {
                for (int x = 0; x < rect.right; ++x) {
                    COLORREF color = GetPixel(o.virtDC, x, y);
                    int brightness = GetRValue(color) + GetGValue(color) + GetBValue(color);
                    if (brightness < 0x200) {
                        // black
                        glyph->set(x, y, true);
                    } else if (brightness < 0x280) {
                        // half
                        glyph->addAAHint(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
                    } else {
                        // white
                    }
                }
            }
            newFont->addNewGlyph(i, glyph.release());
            ++success;
        } else {
            ++errs;
        }

        // Clean up
        SelectObject(o.virtDC, prevBMP);
        DeleteObject(bmp);
    }
    destroyObjects(o);
    if (hGdi) {
        FreeLibrary(hGdi);
    }

    ui::dialogs::MessageBox(afl::string::Format(tx("%d characters imported successfully, %d errors."), success, errs), tx("Font Import"), root)
        .doOkDialog(tx);

    return newFont;
}

#else

/*
 *  Not Windows - Dummy
 */

bool
ui::reshack::Win32Import::isSupported()
{
    return false;
}

afl::base::Ptr<gfx::BitmapFont>
ui::reshack::Win32Import::importFont(Session& /*session*/)
{
    return 0;
}
#endif
