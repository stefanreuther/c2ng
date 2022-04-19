/**
  *  \file game/proxy/costsummaryadaptor.hpp
  *  \brief Class game::proxy::CostSummaryAdaptor
  */
#ifndef C2NG_GAME_PROXY_COSTSUMMARYADAPTOR_HPP
#define C2NG_GAME_PROXY_COSTSUMMARYADAPTOR_HPP

#include "afl/base/closure.hpp"
#include "game/proxy/exportadaptor.hpp"
#include "game/session.hpp"
#include "game/spec/costsummary.hpp"

namespace game { namespace proxy {

    /** ExportAdaptor for a CostSummaryContext.
        Use for exporting a CostSummary.

        @see makeCostSummaryAdaptor() */
    class CostSummaryAdaptor : public ExportAdaptor {
     public:
        /** Constructor.
            @param fs          FileSystem (for fileSystem())
            @param tx          Translator (for translator())
            @param costSummary CostSummary */
        CostSummaryAdaptor(afl::io::FileSystem& fs, afl::string::Translator& tx, afl::base::Ptr<game::spec::CostSummary> costSummary);

        /** Destructor. */
        ~CostSummaryAdaptor();

        // ExportAdaptor:
        virtual void initConfiguration(interpreter::exporter::Configuration& config);
        virtual void saveConfiguration(const interpreter::exporter::Configuration& config);
        virtual interpreter::Context* createContext();
        virtual afl::io::FileSystem& fileSystem();
        virtual afl::string::Translator& translator();

     private:
        afl::io::FileSystem& m_fileSystem;
        afl::string::Translator& m_translator;
        afl::base::Ptr<game::spec::CostSummary> m_costSummary;
    };

    /** Make (creator for) CostSummaryAdaptor.
        Use with RequestSender<Session>::makeTemporary to create a RequestSender<ExportAdaptor>
        that talks to a CostSummaryAdaptor.

        @param costSummary  CostSummary. Will be copied; the copy will be accessible to the game thread.
                            Should not be empty because export cannot handle empty contexts.

        @return Newly-allocated closure */
    afl::base::Closure<ExportAdaptor*(Session&)>* makeCostSummaryAdaptor(const game::spec::CostSummary& costSummary);

} }

#endif
