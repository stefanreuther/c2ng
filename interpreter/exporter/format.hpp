/**
  *  \file interpreter/exporter/format.hpp
  *  \brief Enum interpreter::exporter::Format
  *
  *  FIXME: we need methods to inquire the 'extension' and 'englishDescription' fields.
  *  FIXME: we need a welldefined way to iterate over formats
  */
#ifndef C2NG_INTERPRETER_EXPORTER_FORMAT_HPP
#define C2NG_INTERPRETER_EXPORTER_FORMAT_HPP

#include "afl/string/string.hpp"

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

    /** Get string representation (short name) of a format.
        \param fmt Format
        \return String representation for config file */
    String_t toString(Format fmt);

    /** Parse string representation (short name) into format.
        \param [in] str String
        \param [out] result Result
        \retval true \c str parsed successfully, \c result has been set
        \retval false \c str is not a recognized format */
    bool parseFormat(const String_t& str, Format& result);

} }

#endif
