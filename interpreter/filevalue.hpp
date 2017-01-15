/**
  *  \file interpreter/filevalue.hpp
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
        FileValue(int32_t fileNr);
        ~FileValue();

        virtual String_t toString(bool readable) const;
        virtual FileValue* clone() const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;

        /** Get integer value. */
        int32_t getFileNumber() const
            { return m_fileNr; }

     private:
        int32_t m_fileNr;
    };

    
}

#endif
