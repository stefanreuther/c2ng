/**
  *  \file interpreter/selectionexpression.hpp
  *  \brief Class interpreter::SelectionExpression
  */
#ifndef C2NG_INTERPRETER_SELECTIONEXPRESSION_HPP
#define C2NG_INTERPRETER_SELECTIONEXPRESSION_HPP

#include "afl/string/string.hpp"

namespace interpreter {

    class Tokenizer;

    /** Parser for a selection expression. */
    class SelectionExpression {
     public:
        // These are from GMultiSelection:

        /** Opcodes for compiled selection expressions.
            A selection expression is compiled to a RPN operator string.
            Note that these opcodes are fixed as they are (indirectly) part of process serialisation. */
        static const char opAnd        = '&';     ///< AND. Pop twice, push once.
        static const char opOr         = '|';     ///< OR. Pop twice, push once.
        static const char opXor        = '^';     ///< XOR. Pop twice, push once.
        static const char opNot        = '!';     ///< NOT. Pop once, push once.
        static const char opCurrent    = 'c';     ///< Current. Push once: value of current selection.
        static const char opShip       = 's';     ///< Ship. Push once: true iff this is a ship.
        static const char opPlanet     = 'p';     ///< Planet. Push once: true iff this is a planet.
        static const char opFirstLayer = 'A';     ///< First layer. Other layers obtained by adding the layer number. Pushes value of that layer.
        static const char opZero       = '0';     ///< Zero. Push once: false.
        static const char opOne        = '1';     ///< One. Push once: true.

        /** Number of selection layers.
            Layer numbers are [0,NUM_SELECTION_LAYERS). */
        static const int NUM_SELECTION_LAYERS = 8;

        /** Compile selection expression.
            Selection expressions are compiled into a simple RPN string.

            \param tok [in] Tokenizer to read from
            \param expr [out] Code will be appended here

            expression ::= summand
                         | expression ("+"|"Or") summand
                         | expression "Xor" summand         // this production not in PCC 1.x
                         | expression "-" summand */
        static void compile(Tokenizer& tok, String_t& expr);

     private:
        static void compileOptionalTypeMask(Tokenizer& tok, String_t& expr);
        static void compileFactor(Tokenizer& tok, String_t& expr);
        static void compileSummand(Tokenizer& tok, String_t& expr);
    };

}

#endif
