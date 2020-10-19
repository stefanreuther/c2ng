/**
  *  \file util/plugin/plugin.cpp
  *  \brief Class util::plugin::Plugin
  */

#include "util/plugin/plugin.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/string.hpp"
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

    void addVersion(util::plugin::Plugin::FeatureSet_t& out, String_t comp)
    {
        String_t::size_type n = comp.find_first_of(" \t");
        String_t version;
        if (n != String_t::npos) {
            version = afl::string::strTrim(String_t(comp, n, comp.size()-n));
            comp.erase(n);
        }
        comp = afl::string::strUCase(comp);
        util::plugin::Plugin::FeatureSet_t::iterator it = out.find(comp);
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

    void addVersions(util::plugin::Plugin::FeatureSet_t& out, String_t in)
    {
        String_t::size_type n = 0, p;
        while ((p = in.find(',', n)) != String_t::npos) {
            addVersion(out, afl::string::strTrim(String_t(in, n, p-n)));
            n = p+1;
        }
        addVersion(out, afl::string::strTrim(String_t(in, n, in.size()-n)));
    }

    void writeVersion(afl::io::TextFile& tf, const util::plugin::Plugin::FeatureSet_t::value_type& v)
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

// Constructor.
util::plugin::Plugin::Plugin(String_t id)
    : m_id(id),
      m_name(id),
      m_description(),
      m_baseDir(),
      m_defFileName(),
      m_provides(),
      m_requires(),
      m_items(),
      m_isLoaded(false)
{
    // ex Plugin::Plugin
    m_provides.insert(std::make_pair(id, String_t()));
}

// Destructor.
util::plugin::Plugin::~Plugin()
{ }

// Load plugin definition file (.c2p).
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
                    self.m_name = value;
                } else if (strCaseCompare(name, "description") == 0) {
                    if (!self.m_description.empty()) {
                        self.m_description += "\n";
                    }
                    self.m_description += value;
                } else if (strCaseCompare(name, "provides") == 0) {
                    addVersions(self.m_provides, value);
                } else if (strCaseCompare(name, "requires") == 0) {
                    addVersions(self.m_requires, value);
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

    m_baseDir = baseDir;
    m_defFileName = defFileName;
    Parser p(*this, log);
    p.setSection("plugin", true);
    p.parseFile(file);
}

// Create from resource file.
void
util::plugin::Plugin::initFromResourceFile(String_t baseDir, String_t resFileName, afl::string::Translator& tx)
{
    // ex Plugin::initFromResourceFile
    m_baseDir = baseDir;
    m_defFileName = "";
    m_name = afl::string::strLCase(resFileName);
    m_description = tx("Resource file (artwork)");
    m_items.push_back(Item(ResourceFile, resFileName));
}

// Create from script file.
void
util::plugin::Plugin::initFromScriptFile(String_t baseDir, String_t scriptFileName, afl::io::Stream& file, afl::string::Translator& tx)
{
    // ex Plugin::initFromScriptFile
    m_baseDir = baseDir;
    m_defFileName = "";
    m_name = afl::string::strLCase(scriptFileName);
    m_description = "";
    m_items.push_back(Item(ScriptFile, scriptFileName));

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
                m_name = line;
                state = FindBlank;
            }
            break;

         case FindBlank:
            if (!line.empty()) {
                // We're expecting the blank line after the description,
                // but got a nonblank line. Make it all the description.
                m_description = m_name;
                m_description += " ";
                m_description += line;
            }
            state = FindDescription;
            break;

         case FindDescription:
            if (!line.empty()) {
                if (!m_description.empty()) {
                    m_description += " ";
                }
                m_description += line;
            } else {
                state = Stop;
            }
            break;

         case Stop:
            break;
        }
    }

    // Trim description to two sentences.
    String_t::size_type n = m_description.find(". ");
    if (n != String_t::npos) {
        n = m_description.find(". ", n+1);
    }
    if (n != String_t::npos) {
        m_description.erase(n+1);
    }
    if (m_description.empty()) {
        m_description = tx("Script file");
    }
}

// Create from resource configuration file (cc-res.cfg).
void
util::plugin::Plugin::initFromConfigFile(String_t baseDir, String_t pluginName, afl::io::Stream& file, afl::string::Translator& tx)
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
            m_items.push_back(Item(ResourceFile, line));
        }
    }

    m_name = pluginName;
    m_defFileName = "";
    m_description = tx("Resource configuration file");
    m_baseDir = baseDir;
}

// Save as plugin (.c2p) file.
void
util::plugin::Plugin::savePluginFile(afl::io::Stream& file) const
{
    // ex Plugin::savePluginFile
    afl::io::TextFile tf(file);
    tf.writeLine("# Auto-generated plugin definition file");

    tf.writeText("Name = ");
    tf.writeLine(m_name);

    String_t::size_type n = 0, p;
    while ((p = m_description.find('\n', n)) != String_t::npos) {
        tf.writeText("Description = ");
        tf.writeLine(m_description.substr(n, p-n));
        n = p+1;
    }
    tf.writeText("Description = ");
    tf.writeLine(m_description.substr(n));

    for (FeatureSet_t::const_iterator i = m_provides.begin(); i != m_provides.end(); ++i) {
        if (i->first != m_id) {
            tf.writeText("Provides = ");
            writeVersion(tf, *i);
        }
    }
    for (FeatureSet_t::const_iterator i = m_requires.begin(); i != m_requires.end(); ++i) {
        tf.writeText("Requires = ");
        writeVersion(tf, *i);
    }
    for (ItemList_t::const_iterator i = m_items.begin(); i != m_items.end(); ++i) {
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

// Set base directory.
void
util::plugin::Plugin::setBaseDirectory(const String_t& baseDir)
{
    // ex Plugin::setBaseDirectory
    m_baseDir = baseDir;
}

// Add an item to this plugin.
void
util::plugin::Plugin::addItem(ItemType type, const String_t& name)
{
    // ex Plugin::addItem
    m_items.push_back(Item(type, name));
}

// Set "loaded" flag.
void
util::plugin::Plugin::setLoaded(bool flag)
{
    // ex Plugin::setLoaded
    m_isLoaded = flag;
}

// Get plugin Id.
const String_t&
util::plugin::Plugin::getId() const
{
    // ex Plugin::getId
    return m_id;
}

// Get plugin name.
const String_t&
util::plugin::Plugin::getName() const
{
    // ex Plugin::getName
    return m_name;
}

// Get description.
const String_t&
util::plugin::Plugin::getDescription() const
{
    // ex Plugin::getDescription
    return m_description;
}

// Get base directory.
const String_t&
util::plugin::Plugin::getBaseDirectory() const
{
    // ex Plugin::getBaseDirectory
    return m_baseDir;
}

// Get definition file name.
const String_t&
util::plugin::Plugin::getDefinitionFileName() const
{
    // ex Plugin::getDefinitionFileName
    return m_defFileName;
}

// Get items (files, commands) contained in this plugin.
const util::plugin::Plugin::ItemList_t&
util::plugin::Plugin::getItems() const
{
    // ex Plugin::getItems
    return m_items;
}

// Check whether this plugin provides a certain feature.
bool
util::plugin::Plugin::isProvided(const String_t& feature) const
{
    // ex Plugin::isProvided
    return m_provides.find(feature) != m_provides.end();
}

// Check whether this plugin conflicts with another.
bool
util::plugin::Plugin::isConflict(const Plugin& other) const
{
    // ex Plugin::isConflict
    for (FeatureSet_t::const_iterator pi = m_provides.begin(), pe = m_provides.end(); pi != pe; ++pi) {
        if (other.m_provides.find(pi->first) != other.m_provides.end()) {
            // other plugin provides same feature as we -> reject
            return true;
        }
    }
    return false;
}

// Check whether this plugin qualifies as an update for another plugin.
bool
util::plugin::Plugin::isUpdateFor(const Plugin& other) const
{
    // ex Plugin::isUpdateFor
    // All of our preconditions must already be required by other
    for (FeatureSet_t::const_iterator pi = m_requires.begin(), pe = m_requires.end(); pi != pe; ++pi) {
        FeatureSet_t::const_iterator oi = other.m_requires.find(pi->first);
        if (oi == other.m_requires.end() || compareVersions(oi->second, pi->second)) {
            return false;
        }
    }

    // All of other's features must be provided by us
    for (FeatureSet_t::const_iterator pi = other.m_provides.begin(), pe = other.m_provides.end(); pi != pe; ++pi) {
        FeatureSet_t::const_iterator oi = m_provides.find(pi->first);
        if (oi == m_provides.end() || compareVersions(oi->second, pi->second)) {
            return false;
        }
    }

    return true;
}

// Check whether this plugin depends on another one.
bool
util::plugin::Plugin::isDependingOn(const Plugin& other) const
{
    // ex Plugin::isDependingOn
    // Since only one plugin can provide a feature, this condition is satisfied
    // if one of our required features is provided by %other. There cannot be
    // a different plugin that provides the same feature.
    for (FeatureSet_t::const_iterator pi = m_requires.begin(), pe = m_requires.end(); pi != pe; ++pi) {
        if (other.m_provides.find(pi->first) != other.m_provides.end()) {
            return true;
        }
    }
    return false;
}

// Check whether this plugin depends on another one.
bool
util::plugin::Plugin::isSatisfiedBy(const FeatureSet_t& have) const
{
    // ex Plugin::isSatisfied
    for (FeatureSet_t::const_iterator pi = m_requires.begin(), pe = m_requires.end(); pi != pe; ++pi) {
        FeatureSet_t::const_iterator po = have.find(pi->first);
        if (po == have.end() || compareVersions(po->second, pi->second)) {
            // Feature not available or wrong version
            return false;
        }
    }
    return true;
}

// Check whether plugin is loaded.
bool
util::plugin::Plugin::isLoaded() const
{
    // ex Plugin::isLoaded
    return m_isLoaded;
}

// List missing features.
void
util::plugin::Plugin::enumMissingFeatures(const FeatureSet_t& have, FeatureSet_t& missing) const
{
    // ex Plugin::enumMissingFeatures
    for (FeatureSet_t::const_iterator pi = m_requires.begin(), pe = m_requires.end(); pi != pe; ++pi) {
        FeatureSet_t::const_iterator po = have.find(pi->first);
        if (po == have.end() || compareVersions(po->second, pi->second)) {
            missing.insert(*pi);
        }
    }
}

// List provided features.
void
util::plugin::Plugin::enumProvidedFeatures(FeatureSet_t& have) const
{
    // ex Plugin::addProvidedFeaturesTo
    for (FeatureSet_t::const_iterator pi = m_provides.begin(), pe = m_provides.end(); pi != pe; ++pi) {
        // No version check required; we are the only one who provides this feature,
        // so there cannot be a previous version to upgrade.
        have.insert(*pi);
    }
}

// Compare versions.
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
