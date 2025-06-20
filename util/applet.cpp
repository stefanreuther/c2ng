/**
  *  \file util/applet.cpp
  */

#include "util/applet.hpp"
#include "afl/string/format.hpp"
#include "version.hpp"
#include "util/string.hpp"

using afl::string::Format;

struct util::Applet::Runner::Info {
    String_t name;
    String_t untranslatedInfo;
    std::auto_ptr<Applet> applet;
};

util::Applet::Runner::Runner(String_t untranslatedName, afl::sys::Environment& env, afl::io::FileSystem& fs)
    : Application(env, fs),
      m_applets(),
      m_untranslatedName(untranslatedName)
{ }

util::Applet::Runner::~Runner()
{ }

util::Applet::Runner&
util::Applet::Runner::addNew(String_t name, String_t untranslatedInfo, Applet* p)
{
    std::auto_ptr<Applet> pp(p);
    Info* pi = m_applets.pushBackNew(new Info());
    pi->name = name;
    pi->untranslatedInfo = untranslatedInfo;
    pi->applet = pp;
    return *this;
}

void
util::Applet::Runner::appMain()
{
    afl::string::Translator& tx = translator();
    afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl = environment().getCommandLine();
    String_t appletName;
    if (!cmdl->getNextElement(appletName)) {
        errorExit(Format(tx("no command specified. Use \"%s -h\" for help"), environment().getInvocationName()));
    }

    if (appletName == "-h" || appletName == "-help" || appletName == "--help") {
        showHelp();
    } else if (const Info* p = findApplet(appletName)) {
        exit(p->applet->run(*this, *cmdl));
    } else {
        errorExit(Format(tx("invalid command \"%s\" specified. Use \"%s -h\" for help"), appletName, environment().getInvocationName()));
    }
}

const util::Applet::Runner::Info*
util::Applet::Runner::findApplet(const String_t& appletName) const
{
    for (size_t i = 0, n = m_applets.size(); i < n; ++i) {
        const Info* p = m_applets[i];
        if (p->name == appletName) {
            return p;
        }
    }
    return 0;
}

void
util::Applet::Runner::showHelp()
{
    // Build applet list
    afl::string::Translator& tx = translator();
    String_t appletList;
    for (size_t i = 0, n = m_applets.size(); i < n; ++i) {
        const Info* p = m_applets[i];
        appletList += p->name;
        appletList += "\t";
        appletList += tx(p->untranslatedInfo);
        appletList += "\n";
    }

    // Output
    afl::io::TextWriter& w = standardOutput();
    w.writeLine(Format("%s v%s", tx(m_untranslatedName), PCC2_VERSION));
    w.writeLine(Format(tx("\nUsage: %s APPLET [ARGS]\n\nApplets:\n%s"),
                       environment().getInvocationName(),
                       formatOptions(appletList)));
}
