/**
  *  \file interpreter/blobvalue.hpp
  *  \brief Class interpreter::BlobValue
  */
#ifndef C2NG_INTERPRETER_BLOBVALUE_HPP
#define C2NG_INTERPRETER_BLOBVALUE_HPP

#include "afl/base/growablememory.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter {

    /** Blob value.
        This is the buffer used for file I/O. PCC 1.x used strings instead. */
    class BlobValue : public BaseValue {
     public:
        typedef afl::base::GrowableMemory<uint8_t> Data_t;

        BlobValue();
        ~BlobValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

        // Value:
        virtual BlobValue* clone() const;

        // BlobValue:
        /** Get content.
            For simplicity, we expose the content in modifyable form.
            Care must still be taken to never modify any "live" objects.
            This is intended to modify newly-built objects only. */
        Data_t& data();

     private:
        Data_t m_data;
    };

}

#endif
