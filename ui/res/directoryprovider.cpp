/**
  *  \file ui/res/directoryprovider.cpp
  */

#include "ui/res/directoryprovider.hpp"
#include "afl/io/textfile.hpp"
#include "afl/io/filesystem.hpp"
#include "ui/res/manager.hpp"
#include "afl/string/parse.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/blit.hpp"
#include "util/stringparser.hpp"

namespace {
// /** Convert resource identifier string value into file name template.
//     For example, "foo.bar90" is converted to "foo/bar90.", which tells
//     openResourceFile() to look for a file "bar90" with any extension
//     within directory "foo".
//     \param name [in/out] Resource name */
    String_t convertName(String_t name)
    {
        // FIXME: this is recursive, but can probably be written iterative
        String_t::size_type p = name.find('.');
        if (p != String_t::npos) {
            return afl::io::FileSystem::getInstance().makePathName(name.substr(0, p), convertName(name.substr(p+1)));
        } else {
            return name + ".";
        }
    }

    // /** Add constant to all bytes in a pixmap. It must be a palettized pixmap. */
    void addToPixelValue(gfx::Canvas& can, int toAdd)
    {
        gfx::Point pt = can.getSize();
        for (int y = 0; y < pt.getY(); ++y) {
            int x = 0;
            while (x < pt.getX()) {
                gfx::Color_t colors[256];
                afl::base::Memory<gfx::Color_t> colorMemory(colors);
                colorMemory.trim(pt.getX() - x);
                can.getPixels(gfx::Point(x, y), colorMemory);

                for (size_t i = 0, n = colorMemory.size(); i < n; ++i) {
                    colors[i] += toAdd;
                }
                can.drawPixels(gfx::Point(x, y), colorMemory, gfx::OPAQUE_ALPHA);

                x += colorMemory.size();
            }
        }
    }
}

ui::res::DirectoryProvider::DirectoryProvider(afl::base::Ref<afl::io::Directory> dir)
    : m_directory(dir),
      m_aliasMap()
{
    loadAliases();
}

afl::base::Ptr<gfx::Canvas>
ui::res::DirectoryProvider::loadImage(String_t name, Manager& mgr)
{
    // ex ResDirectory::loadPixmap
    String_t spec;
    AliasMap_t::const_iterator it = m_aliasMap.find("g." + name);
    if (it != m_aliasMap.end()) {
        spec = it->second;
    } else {
        spec = convertName(name);
    }

    // Parse it
    String_t fileName = afl::string::strRTrim(afl::string::strFirst(spec, "|"));
    afl::base::Ptr<afl::io::Stream> stream = openResourceFile(*m_directory, fileName, graphicsSuffixes());
    if (stream.get() == 0) {
        return 0;
    }

    // Load pixmap
    afl::base::Ptr<gfx::Canvas> pix = mgr.loadImage(*stream);
    if (pix.get() == 0) {
        return 0;
    }

    // Close file
    stream.reset();

    // Parse modifications
    while (afl::string::strRemove(spec, "|")) {
        String_t op = afl::string::strTrim(afl::string::strFirst(spec, "|"));

        util::StringParser ops(op);
        if (op.size() == 0) {
            // Blank
        } else if (ops.parseString("size:")) {
            // Resize
            int width = -1, height = -1;
            if (ops.parseString("screen")) {
                width = mgr.getScreenSize().getX();
                height = mgr.getScreenSize().getY();
            } else {
                if (ops.parseInt(width)) {
                    if (ops.parseString("%")) {
                        width = mgr.getScreenSize().getX() * width / 100;
                    }
                }
                if (ops.parseString(",") && ops.parseInt(height)) {
                    if (ops.parseString("%")) {
                        height = mgr.getScreenSize().getY() * height / 100;
                    }
                }
            }

            if (ops.parseEnd() && width > 0 && height > 0) {
                // FIXME: can we optimize by not creating a RGBA pixmap if not needed?
                afl::base::Ref<gfx::Canvas> npix = gfx::RGBAPixmap::create(width, height)->makeCanvas();
                blitStretchRotate(*pix, *npix,
                                  gfx::Rectangle(gfx::Point(), pix->getSize()),
                                  gfx::Rectangle(gfx::Point(), npix->getSize()),
                                  0, 0,
                                  width, 0,
                                  0, height);
                pix = npix.asPtr();
            }
        } else if (ops.parseString("add:")) {
            // Add value to each pixel
            int toAdd;
            if (ops.parseInt(toAdd) && ops.parseEnd() && pix->getBitsPerPixel() == 8) {
                addToPixelValue(*pix, toAdd);
            }
        } else {
            afl::base::Ptr<afl::io::Stream> overlayStream = openResourceFile(*m_directory, op, graphicsSuffixes());
            if (overlayStream.get() != 0) {
                afl::base::Ptr<gfx::Canvas> overlay = mgr.loadImage(*overlayStream);
                if (overlay.get() != 0) {
                    pix->blit(gfx::Point(), *overlay, gfx::Rectangle(gfx::Point(), overlay->getSize()));
                }
            }
        }
    }
    return pix;
}

void
ui::res::DirectoryProvider::loadAliases()
{
    // ex ResDirectory::loadAliases (sort-of)
    afl::base::Ptr<afl::io::Stream> s = m_directory->openFileNT("index.txt", afl::io::FileSystem::OpenRead);
    if (s.get() != 0) {
        afl::io::TextFile tf(*s);
        String_t line;
        while (tf.readLine(line)) {
            // Strip comment
            String_t::size_type p = line.find_first_of(";#");
            if (p != line.npos) {
                line.erase(p);
            }
            line = afl::string::strTrim(line);

            if (!line.empty()) {
                // Parse line
                p = line.find('=');
                if (p != String_t::npos) {
                    m_aliasMap[afl::string::strTrim(line.substr(0, p))] = afl::string::strTrim(line.substr(p+1));
                } else {
                    // FIXME: port this
                    // console.write(LOG_ERROR, format(_("%s:%d: file format error -- line ignored"),
                    //                                 in.getName(), tf.getLineNr()));
                }
            }
        }
    }
}

// ResDirectory::~ResDirectory()
// { }


// SfxSampleData*
// ResDirectory::loadSound(ResId id)
// {
//     string_t name = id.toString();
//     alias_map_t::iterator i = alias_map.find("s." + name);
//     if (i != alias_map.end()) {
//         name = i->second.name;
//     } else {
//         convertName(name);
//     }

//     Ptr<Stream> st = openResourceFile(*dir, name, sfx_suffix_list);
//     if (st) {
//         // FIXME: exceptions?
//         try {
//             return new SfxSampleData(*st);
//         }
//         catch (...)
//         { }
//     }
//     return 0;
// }

// Ptr<GfxAnimation>
// ResDirectory::loadAnimation(ResId /*id*/)
// {
//     return 0;
// }

// Ptr<GfxFont>
// ResDirectory::getFont(ResId id)
// {
//     string_t name = id.toString();
//     alias_map_t::iterator i = alias_map.find("f." + name);
//     if (i != alias_map.end()) {
//         name = i->second.name;
//     } else {
//         convertName(name);
//     }

//     Ptr<Stream> s = openResourceFile(*dir, name, fnt_suffix_list);
//     if (s) {
//         Ptr<GfxBitmapFont> res = new GfxBitmapFont();
//         res->load(*s, 0);
//         return res;
//     } else {
//         return 0;
//     }
// }

// Ptr<Stream>
// ResDirectory::getStream(ResId id)
// {
//     string_t name = id.toString();
//     alias_map_t::iterator i = alias_map.find(name);
//     if (i != alias_map.end()) {
//         name = i->second.name;
//     } else {
//         convertName(name);
//     }
//     return openResourceFile(*dir, name, file_suffix_list);
// }
