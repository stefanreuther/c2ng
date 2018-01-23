/**
  *  \file server/format/format.cpp
  */

#include <stdexcept>
#include <memory>
#include "server/format/format.hpp"
#include "afl/base/deleter.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/data/defaultvaluefactory.hpp"
#include "afl/io/bufferedstream.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalsink.hpp"
#include "afl/io/json/parser.hpp"
#include "afl/io/json/writer.hpp"
#include "server/format/beampacker.hpp"
#include "server/format/enginepacker.hpp"
#include "server/format/hullpacker.hpp"
#include "server/format/packer.hpp"
#include "server/format/simpacker.hpp"
#include "server/format/stringpacker.hpp"
#include "server/format/torpedopacker.hpp"
#include "server/format/truehullpacker.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "util/charsetfactory.hpp"

namespace {
    server::format::Packer& makePacker(const String_t& formatName, afl::base::Deleter& del)
    {
        // ex planetscentral/format/format.cc:createPacker
        /* @type Format
           Specifies the data format to work on.
           Could be one of:
           - %string: data is a single string
           - %engspec: "engspec.dat" file (array of engine specifications)
           - %torpspec: "torpspec.dat" file (array of torpedo specifications)
           - %beamspec: "beamspec.dat" file (array of beam weapon specifications)
           - %hullspec: "hullspec.dat" file (array of hull specifications)
           - %truehull: "truehull.dat" file (player/hull assignments)
           - %sim: "*.ccb" file (battle simulation) */
        if (formatName == "string") {
            return del.addNew(new server::format::StringPacker());
        } else if (formatName == "engspec") {
            return del.addNew(new server::format::EnginePacker());
        } else if (formatName == "torpspec") {
            return del.addNew(new server::format::TorpedoPacker());
        } else if (formatName == "beamspec") {
            return del.addNew(new server::format::BeamPacker());
        } else if (formatName == "hullspec") {
            return del.addNew(new server::format::HullPacker());
        } else if (formatName == "truehull") {
            return del.addNew(new server::format::TruehullPacker());
        } else if (formatName == "sim") {
            return del.addNew(new server::format::SimPacker());
        } else {
            throw std::runtime_error(server::INVALID_FILE_TYPE);
        }
    }

    bool makeJsonFlag(const afl::base::Optional<String_t>& format)
    {
        if (const String_t* p = format.get()) {
            if (*p == "json") {
                return true;
            } else if (*p == "obj") {
                return false;
            } else {
                throw std::runtime_error(server::INVALID_DATA_TYPE);
            }
        } else {
            return false;
        }
    }

    afl::charset::Charset& makeCharset(const afl::base::Optional<String_t>& cs, afl::base::Deleter& del)
    {
        if (const String_t* p = cs.get()) {
            afl::charset::Charset* result = util::CharsetFactory().createCharset(*p);
            if (!result) {
                throw std::runtime_error(server::INVALID_CHARSET);
            }
            return del.addNew(result);
        } else {
            return del.addNew(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
        }
    }

}

server::format::Format::Format()
{ }

server::format::Format::~Format()
{ }

afl::data::Value*
server::format::Format::pack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
{
    // Parse parameters
    afl::base::Deleter del;
    Packer& p = makePacker(formatName, del);
    bool flag = makeJsonFlag(format);
    afl::charset::Charset& cs = makeCharset(charset, del);

    // Convert data from JSON if desired
    if (flag) {
        String_t s = toString(data);
        afl::io::ConstMemoryStream cms(afl::string::toBytes(s));
        afl::io::BufferedStream bs(cms);
        afl::data::DefaultValueFactory vf;
        afl::data::Value* parsed = afl::io::json::Parser(bs, vf).parseComplete();
        if (parsed) {
            del.addNew(parsed);
        }
        data = parsed;
    }

    // Do it
    return makeStringValue(p.pack(data, cs));
}

afl::data::Value*
server::format::Format::unpack(String_t formatName, afl::data::Value* data, afl::base::Optional<String_t> format, afl::base::Optional<String_t> charset)
{
    // Parse parameters
    afl::base::Deleter del;
    Packer& p = makePacker(formatName, del);
    bool flag = makeJsonFlag(format);
    afl::charset::Charset& cs = makeCharset(charset, del);

    // Convert
    std::auto_ptr<afl::data::Value> result(p.unpack(toString(data), cs));

    // Convert to JSON if desired
    if (flag) {
        afl::io::InternalSink sink;
        afl::io::json::Writer writer(sink);
        writer.setLineLength(100);
        writer.visit(result.get());
        result.reset(makeStringValue(afl::string::fromBytes(sink.getContent())));
    }
    return result.release();
}
