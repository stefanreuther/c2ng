/**
  *  \file u/helper/contextverifier.hpp
  *  \brief Context verifier
  */
#ifndef C2NG_U_HELPER_CONTEXTVERIFIER_HPP
#define C2NG_U_HELPER_CONTEXTVERIFIER_HPP

#include "interpreter/context.hpp"

/** Context verifier: verifies that types reported in enumProperties() match actual reported data. */
void verifyTypes(interpreter::Context& ctx);

void verifyInteger(interpreter::Context& ctx, const char* name, int value);
void verifyBoolean(interpreter::Context& ctx, const char* name, bool value);
void verifyString(interpreter::Context& ctx, const char* name, const char* value);
void verifyNull(interpreter::Context& ctx, const char* name);

#endif
