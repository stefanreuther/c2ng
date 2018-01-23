/**
  *  \file server/monitor/badnessfileobserver.hpp
  *  \brief Class server::monitor::BadnessFileObserver
  */
#ifndef C2NG_SERVER_MONITOR_BADNESSFILEOBSERVER_HPP
#define C2NG_SERVER_MONITOR_BADNESSFILEOBSERVER_HPP

#include "server/monitor/statusobserver.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace monitor {

    /** Observer for a file storing a service's status.
        The service stores its badness metric ("number of failed connections") in a file.
        The file contains just the number in decimal form.
        The numbers 0 (=all ok) and 1 (=one failure, sporadic) are treated as ok.
        In addition, the file's age is checked; age larger than an hour means the service is probably down. */
    class BadnessFileObserver : public StatusObserver {
     public:
        /** Constructor.
            \param name       User-friendly name
            \param identifier Configuration identifier used for configuring the file name.
                              Note that the BadnessFileObserver starts with no file name configured.
            \param fs         FileSystem instance */
        BadnessFileObserver(String_t name, String_t identifier, afl::io::FileSystem& fs);

        /** Destructor. */
        ~BadnessFileObserver();

        // Observer:
        virtual String_t getName();
        virtual String_t getId();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual Status checkStatus();

     private:
        String_t m_name;
        String_t m_identifier;
        afl::io::FileSystem& m_fileSystem;
        String_t m_fileName;
    };

} }

#endif
