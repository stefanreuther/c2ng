/**
  *  \file gfx/codec/application.cpp
  *  \brief Class gfx::codec::Application
  */

#include "gfx/codec/application.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"
#include "afl/string/translator.hpp"
#include "gfx/codec/bmp.hpp"
#include "gfx/codec/codec.hpp"
#include "gfx/codec/custom.hpp"
#include "util/resourcefilewriter.hpp"
#include "util/string.hpp"
#include "util/stringparser.hpp"
#include "version.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::string::Format;
using afl::string::Translator;
using gfx::codec::Application;
using util::StringParser;

struct gfx::codec::Application::Status {
    std::auto_ptr<Codec> codec;
    Ptr<Stream> stream;
};

gfx::codec::Application::Application(afl::sys::Environment& env, afl::io::FileSystem& fs)
    : util::Application(env, fs)
{ }

void
gfx::codec::Application::appMain()
{
    Translator& tx = translator();
    Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    String_t verb;
    if (!cmdl->getNextElement(verb)) {
        errorExit(Format(tx("no command specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    if (verb.size() > 1 && verb[0] == '-' && verb[1] == '-') {
        verb.erase(0, 1);
    }
    if (verb == "-h" || verb == "-help" || verb == "help") {
        showHelp();
    } else if (verb == "convert") {
        doConvert(*cmdl);
    } else if (verb == "create") {
        doCreateResource(*cmdl);
    } else {
        errorExit(Format(tx("invalid command \"%s\" specified. Use \"%s -h\" for help"), verb, environment().getInvocationName()));
    }
}

void
gfx::codec::Application::showHelp()
{
    Translator& tx = translator();
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(Format(tx("PCC2 Graphics Codec Application v%s - (c) 2024 Stefan Reuther"), PCC2_VERSION));
    w.writeText(Format(tx("\n"
                          "Usage:\n"
                          "  %s [-h]\n"
                          "  %0$s COMMAND [-OPTS]\n\n"
                          "%s"
                          "\n"
                          "Report bugs to <Streu@gmx.de>\n"),
                       environment().getInvocationName(),
                       util::formatOptions(tx("Commands:\n"
                                              "  convert INFILE OUTFILE\n"
                                              "  create FILE.res ID=INFILE...\n"
                                              "\n"
                                              "File specification:\n"
                                              "bmp:PATH.bmp\tBitmap file\n"
                                              "plain8:PATH.cd, custom:PATH.cd\tPlain 8-bit custom codec\n"
                                              "plain4:PATH.cc\tPlain 4-bit custom codec\n"
                                              "packed8:PATH.cd\tPacked 8-bit custom codec\n"
                                              "packed4:PATH.cc\tPacked 4-bit custom codec\n"))));
    exit(0);
}

/* "convert" command: convert image by loading in one codec and saving in another. */
void
gfx::codec::Application::doConvert(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    Translator& tx = translator();
    String_t inFileName;
    if (!cmdl->getNextElement(inFileName)) {
        errorExit(tx("missing input file name"));
    }

    String_t outFileName;
    if (!cmdl->getNextElement(outFileName)) {
        errorExit(tx("missing output file name"));
    }

    String_t dummy;
    if (cmdl->getNextElement(dummy)) {
        errorExit(tx("too many arguments"));
    }

    Status in;
    StringParser inParser(inFileName);
    if (!openInput(in, inParser)) {
        errorExit(Format(tx("unrecognized input file name: %s"), inFileName));
    }
    Ref<Canvas> can = in.codec->load(*in.stream);

    Status out;
    StringParser outParser(outFileName);
    if (!openOutput(out, outParser)) {
        errorExit(Format(tx("unrecognized output file name: %s"), outFileName));
    }
    out.codec->save(*can, *out.stream);
}

/* "create" command: bulk-convert images to custom format and store in a resource file. */
void
gfx::codec::Application::doCreateResource(afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl)
{
    Translator& tx = translator();
    String_t resFileName;
    if (!cmdl->getNextElement(resFileName)) {
        errorExit(tx("missing output file name"));
    }

    util::ResourceFileWriter resFile(fileSystem().openFile(resFileName, FileSystem::Create), tx);
    String_t inFileName;
    while (cmdl->getNextElement(inFileName)) {
        StringParser inParser(inFileName);
        Status in;
        int memberId;
        if (inParser.parseInt(memberId) && memberId > 0 && memberId <= 32767
            && inParser.parseCharacter('=')
            && openInput(in, inParser))
        {
            Ref<Canvas> can = in.codec->load(*in.stream);
            Custom(Custom::FourBit, true)
                .save(*can, *resFile.createMember(static_cast<uint16_t>(memberId)));
            Custom(Custom::EightBit, true)
                .save(*can, *resFile.createMember(static_cast<uint16_t>(memberId + 20000)));
        } else {
            errorExit(Format(tx("unrecognized input file name: %s"), inFileName));
        }
    }
    resFile.finishFile();
}

bool
gfx::codec::Application::openInput(Status& st, util::StringParser& p)
{
    return openFile(st, p, FileSystem::OpenRead);
}

bool
gfx::codec::Application::openOutput(Status& st, util::StringParser& p)
{
    return openFile(st, p, FileSystem::Create);
}

bool
gfx::codec::Application::openFile(Status& st, util::StringParser& p, afl::io::FileSystem::OpenMode mode)
{
    if (p.parseString("plain8:")) {
        st.codec.reset(new Custom(Custom::EightBit, false));
        st.stream = fileSystem().openFile(p.getRemainder(), mode).asPtr();
        return true;
    } else if (p.parseString("plain4:")) {
        st.codec.reset(new Custom(Custom::FourBit, false));
        st.stream = fileSystem().openFile(p.getRemainder(), mode).asPtr();
        return true;
    } else if (p.parseString("packed8:") || p.parseString("custom:")) {
        st.codec.reset(new Custom(Custom::EightBit, true));
        st.stream = fileSystem().openFile(p.getRemainder(), mode).asPtr();
        return true;
    } else if (p.parseString("packed4:")) {
        st.codec.reset(new Custom(Custom::FourBit, true));
        st.stream = fileSystem().openFile(p.getRemainder(), mode).asPtr();
        return true;
    } else if (p.parseString("bmp:")) {
        st.codec.reset(new BMP());
        st.stream = fileSystem().openFile(p.getRemainder(), mode).asPtr();
        return true;
    } else {
        return false;
    }
}
