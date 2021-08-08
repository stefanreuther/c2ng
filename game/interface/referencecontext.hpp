/**
  *  \file game/interface/referencecontext.hpp
  *  \brief Class game::interface::ReferenceContext
  */
#ifndef C2NG_GAME_INTERFACE_REFERENCECONTEXT_HPP
#define C2NG_GAME_INTERFACE_REFERENCECONTEXT_HPP

#include "afl/base/types.hpp"
#include "game/reference.hpp"
#include "game/session.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/singlecontext.hpp"

namespace game { namespace interface {

    /** Maximum Id for a reference.
        We allow more than the usual 10000 (MAX_NUMBER) because Ufo Ids are not limited. */
    const Id_t MAX_REFERENCE_ID = 0x7FFF;

    /** Reference property identifier. */
    enum ReferenceProperty {
        irpLocX,                ///< Loc.X: location.
        irpLocY,                ///< Loc.Y: location.
        irpId,                  ///< Id: object Id.
        irpReferenceName,       ///< Name$: Reference::toString()
        irpPlainName,           ///< Name: Session::getReferenceName(PlainName).
        irpDetailedName,        ///< Name.Full: Session::getReferenceName(DetailedName).
        irpKind,                ///< Kind: getReferenceTypeName().
        irpObject               ///< Object: script object.
    };

    /** Reference context: publish properties of a game::Reference. */
    class ReferenceContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        /** Constructor.
            \param ref Reference to publish
            \param session Game session */
        ReferenceContext(Reference ref, Session& session);

        /** Destructor. */
        ~ReferenceContext();

        // Context:
        virtual ReferenceContext* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result);
        virtual afl::data::Value* get(PropertyIndex_t index);
        virtual ReferenceContext* clone() const;
        virtual game::map::Object* getObject();
        virtual void enumProperties(interpreter::PropertyAcceptor& acceptor);

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, interpreter::SaveContext& ctx) const;

        Reference getReference() const;

     private:
        Reference m_ref;
        Session& m_session;
    };

    /** Get property of a reference.
        \param ref     Reference
        \param prop    Property
        \param session Game session
        \return newly-allocated value */
    afl::data::Value* getReferenceProperty(Reference ref, ReferenceProperty prop, Session& session);

    /** Create context object for a reference.
        Used to implement the "Object" property (irpObject).
        \param ref     Reference
        \param session Session
        \return newly-allocated context value */
    interpreter::Context* makeObjectValue(Reference ref, Session& session);

    /** Format reference type as string.
        \param t Type to format
        \return Name. Can be null if name cannot be produced */
    const char* getReferenceTypeName(Reference::Type t);

    /** Parse reference type from string.
        \param[in] str String to parse
        \param[out] t  Reference type
        \retval true success, t has been updated
        \retval false failure, string could not be parsed, t not changed */
    bool parseReferenceTypeName(const String_t& str, Reference::Type& t);

    /** Implementation of the "Reference()" function.
        For use with SimpleFunction. */
    afl::data::Value* IFReference(game::Session& session, interpreter::Arguments& args);

    /** Implementation of the "LocationReference()" function.
        For use with SimpleFunction. */
    afl::data::Value* IFLocationReference(game::Session& session, interpreter::Arguments& args);

    /** Check argument of type Reference.
        \param[out] out Result
        \param[in] p User-provided value
        \retval true Value was a valid reference
        \retval false Value was empty
        \throw interpreter::Error type error */
    bool checkReferenceArg(Reference& out, afl::data::Value* p);

} }

#endif
