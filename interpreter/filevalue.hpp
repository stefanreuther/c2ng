/**
  *  \file interpreter/filevalue.hpp
  *  \brief Class interpreter::FileValue
  */
#ifndef C2NG_INTERPRETER_FILEVALUE_HPP
#define C2NG_INTERPRETER_FILEVALUE_HPP

#include "interpreter/basevalue.hpp"

namespace interpreter {

    /** File descriptor.
        This is a special type instead of a compiler syntax feature
        because we occasionally hand it into regular functions, which are not compiled specially.
        It's otherwise just an integer. */
    class FileValue : public BaseValue {
     public:
        /** Constructor.
            \param fileNr File number */
        explicit FileValue(int32_t fileNr);

        /** Destructor. */
        ~FileValue();

        /** Get integer value. */
        int32_t getFileNumber() const;

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual FileValue* clone() const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

     private:
        int32_t m_fileNr;
    };

}

inline int32_t
interpreter::FileValue::getFileNumber() const
{
    return m_fileNr;
}

#endif
