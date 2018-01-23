/**
  *  \file server/monitor/loadaverageobserver.hpp
  */
#ifndef C2NG_SERVER_MONITOR_LOADAVERAGEOBSERVER_HPP
#define C2NG_SERVER_MONITOR_LOADAVERAGEOBSERVER_HPP

#include "server/monitor/observer.hpp"
#include "afl/io/filesystem.hpp"

namespace server { namespace monitor {

    class LoadAverageObserver : public Observer {
     public:
        LoadAverageObserver(afl::io::FileSystem& fs, const String_t& fileName);
        virtual String_t getName();
        virtual String_t getId();
        virtual String_t getUnit();
        virtual bool handleConfiguration(const String_t& key, const String_t& value);
        virtual Result check();

     private:
        afl::io::FileSystem& m_fileSystem;
        const String_t m_fileName;
    };

} } 

#endif
