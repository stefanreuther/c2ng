/**
  *  \file game/proxy/exportproxy.hpp
  *  \brief Class game::proxy::ExportProxy
  */
#ifndef C2NG_GAME_PROXY_EXPORTPROXY_HPP
#define C2NG_GAME_PROXY_EXPORTPROXY_HPP

#include "afl/base/signal.hpp"
#include "afl/data/stringlist.hpp"
#include "game/proxy/exportadaptor.hpp"
#include "game/proxy/waitindicator.hpp"
#include "interpreter/exporter/configuration.hpp"
#include "interpreter/exporter/fieldlist.hpp"
#include "util/requestreceiver.hpp"
#include "util/requestsender.hpp"

namespace game { namespace proxy {

    /** Proxy for data export.
        Provides access to a interpreter::exporter::Configuration object and operations on it.

        Bidirectional, synchronous:
        - getStatus()
        - load(), save()
        - exportFile()
        - enumProperties()

        Bidirectional, asynchronous:
        - modify field list and attributes

        The ExportProxy is configured using an ExportAdaptor object.
        The configuration lives only within the proxy, but the ExportAdaptor can preload/persist it.
        Changes to the configuration are reported using sig_change.

        Field names are always reported in canonical (upper-case) format.
        It is up to the user to format them (util::formatName). */
    class ExportProxy {
     public:
        /** Shortcut for a list index. */
        typedef interpreter::exporter::FieldList::Index_t Index_t;

        /** Constructor.
            @param adaptorSender Sender to configure the ExportProxy
            @param receiver RequestDispatcher to receive events */
        ExportProxy(util::RequestSender<ExportAdaptor> adaptorSender, util::RequestDispatcher& receiver);

        /** Destructor. */
        ~ExportProxy();


        /*
         *  Overall Setup and Operation
         */

        /** Get current status.
            Retrieves the current configuration.
            @param [in]  ind     WaitIndicator for UI synchronisation
            @param [out] config  Configuration */
        void getStatus(WaitIndicator& ind, interpreter::exporter::Configuration& config);

        /** Set character set by index.
            Will eventually generate a sig_change with the new configuration.
            @param index Index (see util::CharsetFactory)
            @see interpreter::exporter::Configuration::setCharsetIndex() */
        void setCharsetIndex(util::CharsetFactory::Index_t index);

        /** Set format.
            Will eventually generate a sig_change with the new configuration.
            @param fmt Format
            @see interpreter::exporter::Configuration::setFormat() */
        void setFormat(interpreter::exporter::Format fmt);

        /** Load configuration from file.
            The sig_change callback with the new configuration will arrive asynchronously.
            @param [in]  ind           WaitIndicator for UI synchronisation
            @param [in]  fileName      File name to load
            @param [out] errorMessage  Error message
            @return true on success, false on error (message set)
            @see interpreter::exporter::Configuration::load() */
        bool load(WaitIndicator& ind, String_t fileName, String_t& errorMessage);

        /** Save configuration to file.
            @param [in]  ind           WaitIndicator for UI synchronisation
            @param [in]  fileName      File name to save
            @param [out] errorMessage  Error message
            @return true on success, false on error (message set)
            @see interpreter::exporter::Configuration::save() */
        bool save(WaitIndicator& ind, String_t fileName, String_t& errorMessage);

        /** Perform export into a file.
            @param [in]  ind           WaitIndicator for UI synchronisation
            @param [in]  fileName      File name to save
            @param [out] errorMessage  Error message
            @return true on success, false on error (message set)
            @see interpreter::exporter::Configuration::exportFile() */
        bool exportFile(WaitIndicator& ind, String_t fileName, String_t& errorMessage);


        /*
         *  Field List
         */

        /** Add field.
            This function is not intended to process user input and therefore doesn't verify it.
            The name is supposed to come from a controlled selector.
            Will eventually generate a sig_change with the new configuration.
            @param index Add before this index (0=as new first, size()=as new last)
            @param name Name of field
            @param width Width of field (0=use default)
            @see interpreter::exporter::FieldList::add() */
        void add(Index_t index, String_t name, int width);

        /** Swap fields.
            Will eventually generate a sig_change with the new configuration.
            @param a,b Positions of fields to swap [0,size())
            @see interpreter::exporter::FieldList::swap() */
        void swap(Index_t a, Index_t b);

        /** Delete a field.
            Will eventually generate a sig_change with the new configuration.
            @param index Index to delete [0,size())
            @see interpreter::exporter::FieldList::remove() */
        void remove(Index_t index);

        /** Clear the list.
            Will eventually generate a sig_change with the new configuration.
            @see interpreter::exporter::FieldList::clear() */
        void clear();

        /** Change field name.
            Will eventually generate a sig_change with the new configuration.
            @param index Position of field [0,size())
            @param name New field name
            @see interpreter::exporter::FieldList::setFieldName() */
        void setFieldName(Index_t index, String_t name);

        /** Change width of a field.
            Will eventually generate a sig_change with the new configuration.
            @param index Position of field [0,size())
            @param width New width (0=use default)
            @see interpreter::exporter::FieldList::setFieldWidth() */
        void setFieldWidth(Index_t index, int width);

        /** Change width of a field, relative.
            Will eventually generate a sig_change with the new configuration.
            @param index Position of field [0,size())
            @param delta Change
            @see interpreter::exporter::FieldList::changeFieldWidth() */
        void changeFieldWidth(Index_t index, int delta);

        /** Toggle field's alignment.
            Will eventually generate a sig_change with the new configuration.
            If the field is left-aligned, makes it right-aligned, and vice-versa.
            (This negates the field width.)
            @param index Position of field [0,size())
            @see interpreter::exporter::FieldList::toggleFieldAlignment() */
        void toggleFieldAlignment(Index_t index);


        /*
         *  Adding Fields
         */

        /** Retrieve list of properties.
            @param [in]  ind WaitIndicator
            @param [out] out Result */
        void enumProperties(WaitIndicator& ind, afl::data::StringList_t& out);


        /** Signal: configuration change.
            Called in response to every action that changes the configuration. */
        afl::base::Signal<void(const interpreter::exporter::Configuration&)> sig_change;

     private:
        class Trampoline;
        class TrampolineFromAdaptor;

        util::RequestReceiver<ExportProxy> m_receiver;
        util::RequestSender<Trampoline> m_sender;

        typedef bool (Trampoline::*FileFunction_t)(String_t, String_t&);
        bool callFileFunction(WaitIndicator& ind, String_t fileName, String_t& errorMessage, FileFunction_t fcn);
    };

} }

#endif
