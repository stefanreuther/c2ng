/**
  *  \file util/plugin/plugin.cpp
  */

#include "util/plugin/plugin.hpp"
#include "afl/string/string.hpp"
#include "util/translation.hpp"
#include "afl/io/textfile.hpp"
#include "util/configurationfileparser.hpp"

namespace {
    const char LOG_NAME[] = "plugin";

    bool eatVersion(const String_t& str, String_t::size_type& pos, uint32_t& version)
    {
        bool ok = false;
        version = 0;
        while (pos < str.size() && str[pos] >= '0' && str[pos] <= '9') {
            version = 10*version + (str[pos] - '0');
            ++pos;
            ok = true;
        }
        if (ok && pos < str.size() && str[pos] == '.') {
            ++pos;
        }
        return ok;
    }

    void addVersion(util::plugin::Plugin::FeatureSet& out, String_t comp)
    {
        String_t::size_type n = comp.find_first_of(" \t");
        String_t version;
        if (n != String_t::npos) {
            version = afl::string::strTrim(String_t(comp, n, comp.size()-n));
            comp.erase(n);
        }
        comp = afl::string::strUCase(comp);
        util::plugin::Plugin::FeatureSet::iterator it = out.find(comp);
        if (it != out.end()) {
            // Someone did "required = foo 1.0, foo 2.0". This is stupid.
            // Turn it into "required = foo 2.0".
            if (util::plugin::compareVersions(it->second, version)) {
                it->second = version;
            }
        } else {
            out.insert(std::make_pair(comp, version));
        }
    }

    void addVersions(util::plugin::Plugin::FeatureSet& out, String_t in)
    {
        String_t::size_type n = 0, p;
        while ((p = in.find(',', n)) != String_t::npos) {
            addVersion(out, afl::string::strTrim(String_t(in, n, p-n)));
            n = p+1;
        }
        addVersion(out, afl::string::strTrim(String_t(in, n, in.size()-n)));
    }

    void writeVersion(afl::io::TextFile& tf, const util::plugin::Plugin::FeatureSet::value_type& v)
    {
        tf.writeText(v.first);
        if (v.second.empty()) {
            tf.writeLine();
        } else {
            tf.writeText(" ");
            tf.writeLine(v.second);
        }
    }
}

util::plugin::Plugin::Plugin(String_t id)
    : id(id),
      name(id),
      description(),
      baseDir(),
      defFileName(),
      provides(),
      requires(),
      items(),
      loaded(false)
{
    // ex Plugin::Plugin
    provides.insert(std::make_pair(id, String_t()));
}

util::plugin::Plugin::~Plugin()
{ }

void
util::plugin::Plugin::initFromPluginFile(String_t baseDir, String_t defFileName, afl::io::Stream& file, afl::sys::LogListener& log)
{
    // ex Plugin::initFromPluginFile
    class Parser : public ConfigurationFileParser {
     public:
        Parser(Plugin& self, afl::sys::LogListener& log)
            : self(self),
              m_log(log)
            { }
        virtual void handleAssignment(const String_t& /*fileName*/, int /*lineNr*/, const String_t& name, const String_t& value, const String_t& /*line*/)
            {
                using afl::string::strCaseCompare;
                if (strCaseCompare(name, "name") == 0) {
                    self.name = value;
                } else if (strCaseCompare(name, "description") == 0) {
                    if (!self.description.empty()) {
                        self.description += "\n";
                    }
                    self.description += value;
                } else if (strCaseCompare(name, "provides") == 0) {
                    addVersions(self.provides, value);
                } else if (strCaseCompare(name, "requires") == 0) {
                    addVersions(self.requires, value);
                } else if (strCaseCompare(name, "scriptfile") == 0) {
                    self.addItem(ScriptFile, value);
                } else if (strCaseCompare(name, "resourcefile") == 0) {
                    self.addItem(ResourceFile, value);
                } else if (strCaseCompare(name, "helpfile") == 0) {
                    self.addItem(HelpFile, value);
                } else if (name.size() >= 4 && strCaseCompare(name.substr(name.size()-4), "file") == 0) {
                    self.addItem(PlainFile, value);
                } else if (strCaseCompare(name, "exec") == 0) {
                    self.addItem(Command, value);
                } else {
                    // Ignore unknown key
                }
            }

        virtual void handleError(const String_t& fileName, int lineNr, const String_t& msg)
            { m_log.write(afl::sys::LogListener::Warn, LOG_NAME, fileName, lineNr, msg); }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
     private:
        Plugin& self;
        afl::sys::LogListener& m_log;
    };

    this->baseDir = baseDir;
    this->defFileName = defFileName;
    Parser p(*this, log);
    p.setSection("plugin", true);
    p.parseFile(file);
}

void
util::plugin::Plugin::initFromResourceFile(String_t baseDir, String_t resFileName)
{
    // ex Plugin::initFromResourceFile
    this->baseDir = baseDir;

    // FIXME: what does this comment mean?
    // Need not set this->defFileName which is only needed when we start with a .c2p file
    // this->defFileName = "";

    this->name = afl::string::strLCase(resFileName);
    this->description = _("Resource file (artwork)");
    items.push_back(Item(ResourceFile, resFileName));
}

void
util::plugin::Plugin::initFromScriptFile(String_t baseDir, String_t scriptFileName, afl::io::Stream& file)
{
    // ex Plugin::initFromScriptFile
    this->baseDir = baseDir;

    // FIXME: what does this comment mean?
    // Need not set this->defFileName which is only needed when we start with a .c2p file
    // this->defFileName = "";
    this->name = afl::string::strLCase(scriptFileName);
    this->description = "";
    items.push_back(Item(ScriptFile, scriptFileName));

    // Parse the script and attempt to extract some information.
    afl::io::TextFile tf(file);
    String_t line;
    enum { FindName, FindBlank, FindDescription, Stop } state = FindName;
    while (state != Stop && tf.readLine(line)) {
        // Stop when the top comment ends
        if (line.empty() || line[0] != '%') {
            break;
        }

        // Trim leading space
        size_t n = 1;
        while (line.size() > n && line[n] == ' ') {
            ++n;
        }
        line.erase(0, n);

        switch (state) {
         case FindName:
            if (!line.empty()) {
                this->name = line;
                state = FindBlank;
            }
            break;

         case FindBlank:
            if (!line.empty()) {
                // We're expecting the blank line after the description,
                // but got a nonblank line. Make it all the description.
                this->description = this->name;
                this->description += " ";
                this->description += line;
            }
            state = FindDescription;
            break;

         case FindDescription:
            if (!line.empty()) {
                if (!this->description.empty()) {
                    this->description += " ";
                }
                this->description += line;
            } else {
                state = Stop;
            }
            break;

         case Stop:
            break;
        }
    }

    // Trim description to two sentences.
    String_t::size_type n = description.find(". ");
    if (n != String_t::npos) {
        n = description.find(". ", n+1);
    }
    if (n != String_t::npos) {
        description.erase(n+1);
    }
    if (description.empty()) {
        description = _("Script file");
    }
}

void
util::plugin::Plugin::initFromConfigFile(String_t baseDir, String_t pluginName, afl::io::Stream& file)
{
    // ex resmgr/resmgrcf.cc:loadResourceConfig
    // @change: we accept '#' as comment
    afl::io::TextFile tf(file);
    String_t line;
    while (tf.readLine(line)) {
        String_t::size_type n = line.find_first_of(";#");
        if (n != line.npos) {
            line.erase(n);
        }
        line = afl::string::strTrim(line);
        if (!line.empty()) {
            items.push_back(Item(ResourceFile, line));
        }
    }

    this->name = pluginName;
    this->description = _("Resource configuration file");
    this->baseDir = baseDir;
}

void
util::plugin::Plugin::savePluginFile(afl::io::Stream& file) const
{
    // ex Plugin::savePluginFile
    afl::io::TextFile tf(file);
    tf.writeLine("# Auto-generated plugin definition file");

    tf.writeText("Name = ");
    tf.writeLine(name);

    String_t::size_type n = 0, p;
    while ((p = description.find('\n', n)) != String_t::npos) {
        tf.writeText("Description = ");
        tf.writeLine(description.substr(n, p-n));
        n = p+1;
    }
    tf.writeText("Description = ");
    tf.writeLine(description.substr(n));

    for (FeatureSet::const_iterator i = provides.begin(); i != provides.end(); ++i) {
        if (i->first != id) {
            tf.writeText("Provides = ");
            writeVersion(tf, *i);
        }
    }
    for (FeatureSet::const_iterator i = requires.begin(); i != requires.end(); ++i) {
        tf.writeText("Requires = ");
        writeVersion(tf, *i);
    }
    for (ItemList::const_iterator i = items.begin(); i != items.end(); ++i) {
        switch (i->type) {
         case PlainFile:    tf.writeText("File = ");         break;
         case ScriptFile:   tf.writeText("ScriptFile = ");   break;
         case ResourceFile: tf.writeText("ResourceFile = "); break;
         case HelpFile:     tf.writeText("HelpFile = ");     break;
         case Command:      tf.writeText("Exec = ");         break;
        }
        tf.writeLine(i->name);
    }
    tf.flush();
}

void
util::plugin::Plugin::setBaseDirectory(const String_t& baseDir)
{
    // ex Plugin::setBaseDirectory
    this->baseDir = baseDir;
}

void
util::plugin::Plugin::addItem(ItemType type, const String_t& name)
{
    // ex Plugin::addItem
    items.push_back(Item(type, name));
}

void
util::plugin::Plugin::setLoaded(bool flag)
{
    // ex Plugin::setLoaded
    this->loaded = flag;
}

const String_t&
util::plugin::Plugin::getId() const
{
    // ex Plugin::getId
    return id;
}

const String_t&
util::plugin::Plugin::getName() const
{
    // ex Plugin::getName
    return name;
}

const String_t&
util::plugin::Plugin::getDescription() const
{
    // ex Plugin::getDescription
    return description;
}

const String_t&
util::plugin::Plugin::getBaseDirectory() const
{
    // ex Plugin::getBaseDirectory
    return baseDir;
}

const String_t&
util::plugin::Plugin::getDefinitionFileName() const
{
    // ex Plugin::getDefinitionFileName
    return defFileName;
}

const util::plugin::Plugin::ItemList&
util::plugin::Plugin::getItems() const
{
    // ex Plugin::getItems
    return items;
}

// /** Check whether this plugin provides a feature. */
bool
util::plugin::Plugin::isProvided(const String_t& feature) const
{
    // ex Plugin::isProvided
    return provides.find(feature) != provides.end();
}

// /** Check for installation conflict. */
bool
util::plugin::Plugin::isConflict(const Plugin& other) const
{
    // ex Plugin::isConflict
    for (FeatureSet::const_iterator pi = provides.begin(), pe = provides.end(); pi != pe; ++pi) {
        if (other.provides.find(pi->first) != other.provides.end()) {
            // other plugin provides same feature as we -> reject
            return true;
        }
    }
    return false;
}

// /** Check whether this plugin is a drop-in update for another one.
//     This means it must have the same or fewer preconditions,
//     and provide the same or better features. */
bool
util::plugin::Plugin::isUpdateFor(const Plugin& other) const
{
    // ex Plugin::isUpdateFor
    // All of our preconditions must already be required by other
    for (FeatureSet::const_iterator pi = requires.begin(), pe = requires.end(); pi != pe; ++pi) {
        FeatureSet::const_iterator oi = other.requires.find(pi->first);
        if (oi == other.requires.end() || compareVersions(oi->second, pi->second)) {
            return false;
        }
    }

    // All of other's features must be provided by us
    for (FeatureSet::const_iterator pi = other.provides.begin(), pe = other.provides.end(); pi != pe; ++pi) {
        FeatureSet::const_iterator oi = provides.find(pi->first);
        if (oi == provides.end() || compareVersions(oi->second, pi->second)) {
            return false;
        }
    }

    return true;
}

// /** Check whether this plugin depends on another one. */
bool
util::plugin::Plugin::isDependingOn(const Plugin& other) const
{
    // ex Plugin::isDependingOn
    // Since only one plugin can provide a feature, this condition is satisfied
    // if one of our required features is provided by %other. There cannot be
    // a different plugin that provides the same feature.
    for (FeatureSet::const_iterator pi = requires.begin(), pe = requires.end(); pi != pe; ++pi) {
        if (other.provides.find(pi->first) != other.provides.end()) {
            return true;
        }
    }
    return false;
}

// /** Check whether the given feature set satisfies our requirements. */
bool
util::plugin::Plugin::isSatisfied(const FeatureSet& fset) const
{
    // ex Plugin::isSatisfied
    for (FeatureSet::const_iterator pi = requires.begin(), pe = requires.end(); pi != pe; ++pi) {
        FeatureSet::const_iterator po = fset.find(pi->first);
        if (po == fset.end() || compareVersions(po->second, pi->second)) {
            // Feature not available or wrong version
            return false;
        }
    }
    return true;
}

bool
util::plugin::Plugin::isLoaded() const
{
    // ex Plugin::isLoaded
    return loaded;
}

void
util::plugin::Plugin::enumMissingFeatures(const FeatureSet& have, FeatureSet& missing) const
{
    // ex Plugin::enumMissingFeatures
    for (FeatureSet::const_iterator pi = requires.begin(), pe = requires.end(); pi != pe; ++pi) {
        FeatureSet::const_iterator po = have.find(pi->first);
        if (po == have.end() || compareVersions(po->second, pi->second)) {
            missing.insert(*pi);
        }
    }
}

// /** Add all features we provide to the given feature set. */
void
util::plugin::Plugin::addProvidedFeaturesTo(FeatureSet& fset)
{
    // ex Plugin::addProvidedFeaturesTo
    for (FeatureSet::const_iterator pi = provides.begin(), pe = provides.end(); pi != pe; ++pi) {
        // No version check required; we are the only one who provides this feature,
        // so there cannot be a previous version to upgrade.
        fset.insert(*pi);
    }
}

/** Compare versions.
    \return true iff a is older than b */
bool
util::plugin::compareVersions(const String_t& a, const String_t& b)
{
    String_t::size_type apos = 0, bpos = 0;
    while (1) {
        uint32_t avers, bvers;
        bool aok = eatVersion(a, apos, avers);
        bool bok = eatVersion(b, bpos, bvers);
        if (aok) {
            if (bok) {
                // Two versions.
                if (avers != bvers) {
                    return avers < bvers;
                }
            } else {
                // One version, e.g. "1.1" vs. "1.foo".
                // Treat second one older.
                return false;
            }
        } else {
            if (bok) {
                // One version, e.g. "1.foo" vs. "1.1"
                // Treat first one as older.
                return true;
            } else {
                // No version, e.g. "1.a" vs. "1.b".
                // String compare.
                return a.substr(apos) < b.substr(bpos);
            }
        }
    }
}
