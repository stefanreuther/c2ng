/**
  *  \file interpreter/exporter/format.cpp
  *  \brief Enum interpreter::exporter::Format
  */

#include "interpreter/exporter/format.hpp"
#include "afl/base/countof.hpp"
#include "util/translation.hpp"

namespace {
    struct Map {
        char name[6];
        char extension[5];
        const char* englishDescription;
    };

    // ex type_names, type_codes, type_extensions
    const Map MAP[] = {
        { "text",  "txt",  N_("Text file") },
        { "table", "txt",  N_("Text table") },
        { "csv",   "csv",  N_("Comma-separated values") },
        { "tsv",   "csv",  N_("Tab-separated values") },
        { "ssv",   "ssv",  N_("Semicolon-separated values") },
        { "json",  "js",   N_("JSON (JavaScript)") },
        { "html",  "html", N_("HTML table") },
        { "dbf",   "dbf",  N_("dBASE file (*.dbf)") },
    };
}


String_t
interpreter::exporter::toString(Format fmt)
{
    return MAP[fmt].name;
}

String_t
interpreter::exporter::getFileNameExtension(Format fmt)
{
    // ex WExportFormatControl::getRecommendedFileExtension
    return MAP[fmt].extension;
}

String_t
interpreter::exporter::getFormatDescription(Format fmt, afl::string::Translator& tx)
{
    return tx(MAP[fmt].englishDescription);
}

bool
interpreter::exporter::parseFormat(const String_t& str, Format& result)
{
    // ex parseOutputFormatType
    for (size_t i = 0; i < countof(MAP); ++i) {
        if (afl::string::strCaseCompare(str, MAP[i].name) == 0) {
            result = Format(i);
            return true;
        }
    }
    return false;
}
