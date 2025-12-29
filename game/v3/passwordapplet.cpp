/**
  *  \file game/v3/passwordapplet.cpp
  *  \brief Class game::v3::PasswordApplet
  */

#include "game/v3/passwordapplet.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "game/v3/genfile.hpp"
#include "game/v3/resultfile.hpp"
#include "game/v3/structures.hpp"
#include "game/v3/turnfile.hpp"

namespace {
    /*
     *  Password decoding logic
     *
     *  Partially reimplemented from GenFile
     */

    const size_t LEN = 10;
    const uint8_t NEW_PASSWORD_OFFSET = 50;
    const int16_t NEW_PASSWORD_FLAG = 13;

    /* Get password from buffer */
    String_t getPassword(uint8_t (&buf)[LEN])
    {
        return afl::string::strTrim(afl::string::fromBytes(buf));
    }

    /* Show password from GenFile structure */
    void showPassword(afl::io::TextWriter& out, game::v3::GenFile& gen)
    {
        game::v3::structures::Gen data;
        gen.getData(data);

        uint8_t requiredPassword[LEN];
        for (size_t i = 0; i < LEN; ++i) {
            uint8_t k = static_cast<uint8_t>(data.password[i] - data.password[2*LEN-1-i] + 32);
            if (k < 32 || k > 127) {
                requiredPassword[i] = ' ';
            } else {
                requiredPassword[i] = k;
            }
        }
        out.writeLine("\tpassword = " + getPassword(requiredPassword));

        if (data.newPasswordFlag == NEW_PASSWORD_FLAG) {
            for (size_t i = 0; i < LEN; ++i) {
                requiredPassword[i] = data.newPassword[i] - NEW_PASSWORD_OFFSET;
            }
            out.writeLine("\tnew password = " + getPassword(requiredPassword));
        }
    }

    /* File extension check */
    bool endsWith(const String_t& big, const String_t& little)
    {
        return big.size() >= little.size()
            && afl::string::strCaseCompare(big.substr(big.size() - little.size()), little) == 0;
    }
}

int
game::v3::PasswordApplet::run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
{
    String_t fileName;
    while (cmdl.getNextElement(fileName)) {
        afl::base::Ref<afl::io::Stream> file = app.fileSystem().openFile(fileName, afl::io::FileSystem::OpenRead);
        handleFile(app.standardOutput(), *file, fileName, app.translator());
    }
    return 0;
}

void
game::v3::PasswordApplet::handleFile(afl::io::TextWriter& out, afl::io::Stream& in, const String_t& fileName, afl::string::Translator& tx)
{
    out.writeLine(fileName + ":");
    if (endsWith(fileName, ".rst")) {
        // Load RST
        ResultFile rst(in, tx);
        rst.seekToSection(ResultFile::GenSection);

        // Parse into GenFile
        GenFile gen;
        gen.loadFromResult(in);
        showPassword(out, gen);
    } else if (endsWith(fileName, ".trn")) {
        // Load Turn
        afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
        TurnFile trn(cs, tx, in);

        // Check for ChangePassword command
        for (size_t i = 0, n = trn.getNumCommands(); i < n; ++i) {
            TurnFile::CommandCode_t cc;
            if (trn.getCommandCode(i, cc) && cc == tcm_ChangePassword) {
                afl::base::ConstBytes_t data = trn.getCommandData(i);
                uint8_t buf[LEN];
                for (size_t i = 0; i < LEN; ++i) {
                    const uint8_t* p = data.eat();
                    buf[i] = p != 0 ? (*p - NEW_PASSWORD_OFFSET) : ' ';
                }
                out.writeLine("\tnew password = " + getPassword(buf));
            }
        }
    } else {
        // Load as GenFile
        GenFile gen;
        gen.loadFromFile(in);
        showPassword(out, gen);
    }
}
