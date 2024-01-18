/**
  *  \file test/interpreter/nametabletest.cpp
  *  \brief Test for interpreter::NameTable
  */

#include "interpreter/nametable.hpp"

#include "afl/test/testrunner.hpp"
#include <cstdio>               // sprintf

/** Test lookupName(). */
AFL_TEST("interpreter.NameTable:lookup", a)
{
    static const interpreter::NameTable tab[] = {
        { "B", 0, 0, interpreter::thNone },
        { "C", 0, 0, interpreter::thNone },
        { "D", 0, 0, interpreter::thNone },
        { "E", 0, 0, interpreter::thNone },
        { "F", 0, 0, interpreter::thNone },
        { "G", 0, 0, interpreter::thNone },
        { "H", 0, 0, interpreter::thNone },
        { "I", 0, 0, interpreter::thNone },
        { "J", 0, 0, interpreter::thNone },
        { "K", 0, 0, interpreter::thNone },
    };
    const size_t SIZE = sizeof(tab) / sizeof(tab[0]);

    static const char*const failTab[] = {
        "A",
        "B1",
        "F1",
        "K1",
        "Z"
    };

    typedef afl::base::Memory<const interpreter::NameTable> NameTable_t;
    for (size_t start = 0; start < SIZE; ++start) {
        for (size_t end = start; end < SIZE; ++end) {
            // Test all elements in the table; they must be found correctly
            interpreter::Context::PropertyIndex_t index;
            for (size_t i = 0; i < SIZE; ++i) {
                char testCaseName[100];
                std::sprintf(testCaseName, "'%s' in [%d,%d)", tab[i].name, int(start), int(end));
                // int32_t result = interpreter::lookupName(tab[i].name, &tab[start], end - start);
                if (i < start || i >= end) {
                    a(testCaseName).check("lookupName", !interpreter::lookupName(tab[i].name, NameTable_t(tab).subrange(start, end - start), index));
                } else {
                    a(testCaseName).check("lookupName", interpreter::lookupName(tab[i].name, NameTable_t(tab).subrange(start, end - start), index));
                    a(testCaseName).checkEqual("index", index, i - start);
                }

            }

            // Test elements that should not be found
            for (size_t i = 0; i < sizeof(failTab)/sizeof(failTab[0]); ++i) {
                char testCaseName[100];
                std::sprintf(testCaseName, "'%s' in [%d,%d)", tab[i].name, int(start), int(end));
                a(testCaseName).check("fail", !interpreter::lookupName(failTab[i], NameTable_t(tab).subrange(start, end - start), index));
            }
        }
    }
}
