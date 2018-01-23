/**
  *  \file interpreter/exporter/configuration.cpp
  */

#include "interpreter/exporter/configuration.hpp"
#include "util/configurationfileparser.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"
#include "util/translation.hpp"

namespace {
    class ExportConfigurationParser : public util::ConfigurationFileParser {
     public:
        ExportConfigurationParser(interpreter::exporter::Configuration& parent)
            : ConfigurationFileParser(),
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
                        m_parent.setFormatByName(value);
                    } else if (afl::string::strCaseCompare(name, "Charset") == 0) {
                        m_parent.setCharsetByName(value);
                    } else {
                        // nix
                    }
                }
                catch (std::exception& e) {
                    throw afl::except::FileProblemException(fileName, afl::string::Format(_("%s (in line %d)").c_str(), e.what(), lineNr));
                }
            }

        virtual void handleError(const String_t& fileName, int lineNr, const String_t& message)
            {
                throw afl::except::FileProblemException(fileName, afl::string::Format(_("%s (in line %d)").c_str(), message, lineNr));
            }

        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
     private:
        interpreter::exporter::Configuration& m_parent;
    };
}


interpreter::exporter::Configuration::Configuration()
    : m_charsetIndex(util::CharsetFactory::LATIN1_INDEX),
      m_format(TextFormat),
      m_fieldList()
{ }
      

interpreter::exporter::Configuration::~Configuration()
{ }

void
interpreter::exporter::Configuration::setCharsetIndex(util::CharsetFactory::Index_t index)
{
    m_charsetIndex = index;
}

void
interpreter::exporter::Configuration::setCharsetByName(const String_t& name)
{
    if (!util::CharsetFactory().findIndexByKey(name, m_charsetIndex)) {
        throw std::runtime_error(_("the specified character set is not known"));
    }
}

util::CharsetFactory::Index_t
interpreter::exporter::Configuration::getCharsetIndex() const
{
    return m_charsetIndex;
}

afl::charset::Charset*
interpreter::exporter::Configuration::createCharset() const
{
    return util::CharsetFactory().createCharsetByIndex(m_charsetIndex);
}

void
interpreter::exporter::Configuration::setFormat(Format fmt)
{
    m_format = fmt;
}

void
interpreter::exporter::Configuration::setFormatByName(const String_t& name)
{
    if (!parseFormat(name, m_format)) {
        throw std::runtime_error(_("invalid output format specified"));
    }
}

interpreter::exporter::Format
interpreter::exporter::Configuration::getFormat() const
{
    return m_format;
}

interpreter::exporter::FieldList&
interpreter::exporter::Configuration::fieldList()
{
    return m_fieldList;
}

const interpreter::exporter::FieldList&
interpreter::exporter::Configuration::fieldList() const
{
    return m_fieldList;
}

void
interpreter::exporter::Configuration::load(afl::io::Stream& in)
{
    ExportConfigurationParser(*this).parseFile(in);
}
