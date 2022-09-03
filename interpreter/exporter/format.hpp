/**
  *  \file interpreter/exporter/format.hpp
  *  \brief Enum interpreter::exporter::Format
  */
#ifndef C2NG_INTERPRETER_EXPORTER_FORMAT_HPP
#define C2NG_INTERPRETER_EXPORTER_FORMAT_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"

namespace interpreter { namespace exporter {

    /** Export format selection.
        Formats are implemented as different Exporter descendants.
        This enum is used to give users a common repertoire of formats to choose from. */
    enum Format {
        TextFormat,
        TableFormat,
        CommaSVFormat,
        TabSVFormat,
        SemicolonSVFormat,
        JSONFormat,
        HTMLFormat,
        DBaseFormat
    };
    const size_t NUM_FORMATS = static_cast<size_t>(DBaseFormat) + 1;

    /** Get string representation (short name) of a format.
        @param fmt Format
        @return String representation for config file */
    String_t toString(Format fmt);

    /** Get preferred file extension for a file format.
        @param fmt Format
        @return Extension, without leading dot */
    String_t getFileNameExtension(Format fmt);

    /** Get human-readable description of file format.
        @param fmt Format
        @param tx  Translator
        @return Description */
    String_t getFormatDescription(Format fmt, afl::string::Translator& tx);

    /** Parse string representation (short name) into format.
        @param [in] str String
        @param [out] result Result
        @retval true @c str parsed successfully, @c result has been set
        @retval false @c str is not a recognized format */
    bool parseFormat(const String_t& str, Format& result);

} }

#endif
