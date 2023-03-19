/**
  *  \file interpreter/exporter/configuration.cpp
  *  \brief Class interpreter::exporter::Configuration
  */

#include "interpreter/exporter/configuration.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/format.hpp"
#include "interpreter/exporter/dbfexporter.hpp"
#include "interpreter/exporter/htmlexporter.hpp"
#include "interpreter/exporter/jsonexporter.hpp"
#include "interpreter/exporter/separatedtextexporter.hpp"
#include "interpreter/exporter/textexporter.hpp"
#include "util/configurationfileparser.hpp"
#include "util/string.hpp"

namespace {
    class ExportConfigurationParser : public util::ConfigurationFileParser {
     public:
        ExportConfigurationParser(interpreter::exporter::Configuration& parent, afl::string::Translator& tx)
            : ConfigurationFileParser(tx),
              m_parent(parent)
            {
                setSection("export", true);
            }

        virtual void handleAssignment(const String_t& fileName, int lineNr, const String_t& name, const String_t& value, const String_t& /*line*/)
            {
                // ex IntExportConfigParser::assign
                try {
                    if (afl::string::strCaseCompare(name, "Fields") == 0) {
                        m_parent.fieldList().addList(value);
                    } else if (afl::string::strCaseCompare(name, "Format") == 0) {
                        m_parent.setFormatByName(value, translator());
                    } else if (afl::string::strCaseCompare(name, "Charset") == 0) {
                        m_parent.setCharsetByName(value, translator());
                    } else {
                        // nix
                    }
                }
                catch (std::exception& e) {
                    throw afl::except::FileProblemException(fileName, afl::string::Format(translator()("%s (in line %d)"), e.what(), lineNr));
                }
            }

        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message)
            {
                throw afl::except::FileProblemException(fileName, afl::string::Format(translator()("%s (in line %d)"), message, lineNr));
            }

        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
     private:
        interpreter::exporter::Configuration& m_parent;
    };
}


// Constructor.
interpreter::exporter::Configuration::Configuration()
    : m_charsetIndex(util::CharsetFactory::LATIN1_INDEX),
      m_format(TextFormat),
      m_fieldList()
{ }

// Destructor.
interpreter::exporter::Configuration::~Configuration()
{ }

// Set character set by index.
void
interpreter::exporter::Configuration::setCharsetIndex(util::CharsetFactory::Index_t index)
{
    m_charsetIndex = index;
}

// Set character set by name.
void
interpreter::exporter::Configuration::setCharsetByName(const String_t& name, afl::string::Translator& tx)
{
    if (!util::CharsetFactory().findIndexByKey(name).get(m_charsetIndex)) {
        throw std::runtime_error(tx("the specified character set is not known"));
    }
}

// Get character set index.
util::CharsetFactory::Index_t
interpreter::exporter::Configuration::getCharsetIndex() const
{
    return m_charsetIndex;
}

// Create configured character set.
afl::charset::Charset*
interpreter::exporter::Configuration::createCharset() const
{
    return util::CharsetFactory().createCharsetByIndex(m_charsetIndex);
}

// Set format.
void
interpreter::exporter::Configuration::setFormat(Format fmt)
{
    m_format = fmt;
}

// Set format by name.
void
interpreter::exporter::Configuration::setFormatByName(const String_t& name, afl::string::Translator& tx)
{
    if (!parseFormat(name, m_format)) {
        throw std::runtime_error(tx("invalid output format specified"));
    }
}

// Get format.
interpreter::exporter::Format
interpreter::exporter::Configuration::getFormat() const
{
    return m_format;
}

// Access field list.
interpreter::exporter::FieldList&
interpreter::exporter::Configuration::fieldList()
{
    return m_fieldList;
}

// Access field list.
const interpreter::exporter::FieldList&
interpreter::exporter::Configuration::fieldList() const
{
    return m_fieldList;
}

// Read configuration from stream.
void
interpreter::exporter::Configuration::load(afl::io::Stream& in, afl::string::Translator& tx)
{
    ExportConfigurationParser(*this, tx).parseFile(in);
}

// Write configuration to stream.
void
interpreter::exporter::Configuration::save(afl::io::Stream& out)
{
    afl::io::TextFile tf(out);
    for (size_t i = 0, n = m_fieldList.size(); i < n; ++i) {
        const int fw = m_fieldList.getFieldWidth(i);
        const String_t name = util::formatName(m_fieldList.getFieldName(i));
        if (fw != 0) {
            tf.writeLine(afl::string::Format("Fields=%s@%d", name, fw));
        } else {
            tf.writeLine(afl::string::Format("Fields=%s", name));
        }
    }
    tf.writeLine(afl::string::Format("Charset=%s", util::CharsetFactory().getCharsetKey(m_charsetIndex)));
    tf.writeLine(afl::string::Format("Format=%s", toString(m_format)));
    tf.flush();
}

// Perform export in a text format.
bool
interpreter::exporter::Configuration::exportText(Context& ctx, afl::io::TextWriter& out) const
{
    switch (m_format) {
     case TextFormat:
        TextExporter(out, false).doExport(ctx, m_fieldList);
        return true;
     case TableFormat:
        TextExporter(out, true).doExport(ctx, m_fieldList);
        return true;
     case CommaSVFormat:
        SeparatedTextExporter(out, ',').doExport(ctx, m_fieldList);
        return true;
     case TabSVFormat:
        SeparatedTextExporter(out, '\t').doExport(ctx, m_fieldList);
        return true;
     case SemicolonSVFormat:
        SeparatedTextExporter(out, ';').doExport(ctx, m_fieldList);
        return true;
     case JSONFormat:
        JsonExporter(out).doExport(ctx, m_fieldList);
        return true;
     case HTMLFormat:
        HtmlExporter(out).doExport(ctx, m_fieldList);
        return true;
     case DBaseFormat:
        /* not a text format */
        break;
    }
    return false;
}

// Perform output into a file.
void
interpreter::exporter::Configuration::exportFile(Context& ctx, afl::io::Stream& out) const
{
    // ex WExportFormatControl::doExport
    const char*const LOCATION = "<Configuration::exportFile>";

    // Must have a charset
    std::auto_ptr<afl::charset::Charset> cs(createCharset());
    afl::except::checkAssertion(cs.get() != 0, "charset", LOCATION);

    // Export
    if (m_format == DBaseFormat) {
        DbfExporter(out, *cs).doExport(ctx, m_fieldList);
    } else {
        afl::io::TextFile tf(out);
        tf.setCharsetNew(cs.release());
        bool ok = exportText(ctx, tf);
        tf.flush();
        afl::except::checkAssertion(ok, "ok", LOCATION);
    }
}
