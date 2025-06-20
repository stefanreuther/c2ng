/**
  *  \file gfx/applet.cpp
  *  \brief Class gfx::Applet
  */

#include "gfx/applet.hpp"
#include "afl/string/format.hpp"
#include "util/string.hpp"

using afl::string::Format;

struct gfx::Applet::Runner::Info {
    String_t name;
    String_t untranslatedInfo;
    std::auto_ptr<Applet> applet;
};

gfx::Applet::Runner::Runner(afl::sys::Dialog& dialog, afl::sys::Environment& env, afl::io::FileSystem& fs, const String_t& title)
    : NullTranslator(),
      Application(dialog, *this, title),
      m_environment(env),
      m_fileSystem(fs),
      m_title(title)
{ }

gfx::Applet::Runner::~Runner()
{ }

gfx::Applet::Runner&
gfx::Applet::Runner::addNew(String_t name, String_t untranslatedInfo, Applet* p)
{
    std::auto_ptr<Applet> pp(p);
    Info* pi = m_applets.pushBackNew(new Info());
    pi->name = name;
    pi->untranslatedInfo = untranslatedInfo;
    pi->applet = pp;
    return *this;
}

void
gfx::Applet::Runner::appMain(Engine& engine)
{
    afl::string::Translator& tx = translator();
    afl::base::Ref<afl::sys::Environment::CommandLine_t> cmdl = m_environment.getCommandLine();
    String_t appletName;
    if (!cmdl->getNextElement(appletName)) {
        dialog().showError(Format(tx("no command specified. Use \"%s -h\" for help"), m_environment.getInvocationName()), m_title);
        exit(1);
    }

    if (appletName == "-h" || appletName == "-help" || appletName == "--help") {
        showHelp();
    } else if (const Info* p = findApplet(appletName)) {
        exit(p->applet->run(*this, engine, m_environment, m_fileSystem, *cmdl));
    } else {
        dialog().showError(Format(tx("invalid command \"%s\" specified. Use \"%s -h\" for help"), appletName, m_environment.getInvocationName()), m_title);
        exit(1);
    }
}

const gfx::Applet::Runner::Info*
gfx::Applet::Runner::findApplet(const String_t& appletName) const
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
gfx::Applet::Runner::showHelp()
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
    dialog().showInfo(Format(tx("\nUsage: %s APPLET [ARGS]\n\nApplets:\n%s"),
                             m_environment.getInvocationName(),
                             util::formatOptions(appletList)), m_title);
    exit(0);
}
