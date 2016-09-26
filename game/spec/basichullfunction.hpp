/**
  *  \file game/spec/basichullfunction.hpp
  */
#ifndef C2NG_GAME_SPEC_BASICHULLFUNCTION_HPP
#define C2NG_GAME_SPEC_BASICHULLFUNCTION_HPP

#include "afl/string/string.hpp"

namespace game { namespace spec {

    /** Basic hull function.
        This defines a basic hull function as known to the host.
        We read their definition from a definition file,
        to allow easier upgrade (and storage of friendly help texts) for future functions. */
    class BasicHullFunction {
     public:
        /** Constructor.
            \param id     [in] Id under which host refers to this function
            \param name   [in] Name (identifier) */
        BasicHullFunction(int id, String_t name);

        /** Destructor. */
        ~BasicHullFunction();

        /** Set function name.
            \param name Name */
        void setName(const String_t& name);

        /** Set short description of function.
            This is what we show to users, a short one-liner.
            \param description new description */
        void setDescription(const String_t& description);

        /** Set explanation of function.
            This is the detailed explanation shown upon user request.
            It can contain multiple lines.
            \param explanation new explanation */
        void setExplanation(const String_t& explanation);

        /** Add to explanation.
            Adds a new line to the existing explanation.
            \param explanation line to add (may include '\\n' but doesn't have to)
            \see setExplanation */
        void addToExplanation(const String_t& explanation);

        /** Set implied function Id.
            Each function can imply another one (usually a lesser version of it),
            meaning that a ship having both will perform only the better one, or, in other words,
            a ship having the better one can also do what the lesser one does.
            \param impliedFunctionId Id of implied function, see getId(). Use -1 for no implied function (default). */
        void setImpliedFunctionId(int impliedFunctionId);

        /** Get function Id.
            \return function Id */
        int getId() const;

        /** Get function name.
            \return function name, see setName() */
        const String_t& getName() const;

        /** Get function description.
            \return description, see setDescription() */
        const String_t& getDescription() const;

        /** Get explanation.
            \return explanation, see setExplanation() */
        const String_t& getExplanation() const;

        /** Get implied function Id.
            \return implied function Id, -1 if none; see setImpliedFunctionId(). */
        int getImpliedFunctionId() const;

     private:
        const int m_id;
        String_t m_name;
        String_t m_description;
        String_t m_explanation;
        int m_impliedFunctionId;
    };

} }

#endif
