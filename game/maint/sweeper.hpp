/**
  *  \file game/maint/sweeper.hpp
  *  \brief Class game::maint::Sweeper
  */
#ifndef C2NG_GAME_MAINT_SWEEPER_HPP
#define C2NG_GAME_MAINT_SWEEPER_HPP

#include "afl/io/directory.hpp"
#include "game/playerset.hpp"

namespace game { namespace maint {

    /** Directory cleaner.
        This implements the core of a "sweep" utility:
        - scan a directory to find active players
        - selectively remove some players' files, updating the index file

        c2ng change: While PCC2 worked on a special interface, this works on a Directory object.
        To achieve the functionality PCC2 obtained using the special interface (logging, dry-run),
        use DirectoryWrapper. */
    class Sweeper {
     public:
        /** Constructor. */
        Sweeper();

        /** Scan game directory.
            This looks for genX.dat files to figure out what players are there.
            This is the same criterion game::v3::DirectoryScanner uses.
            However, unlike DirectoryScanner, this does not look into the files.
            \param dir Scan directory */
        void scan(afl::io::Directory& dir);

        /** Execute operation.
            This removes all selected files and updates the index file (init.tmp) if needed.
            \param dir Directory. If scan() has previously been called, this should be the same directory as given to scan(). */
        void execute(afl::io::Directory& dir);

        /** Configuration: erase database flag.
            If set, deletes files usually kept longer (databases) which cannot be recovered by unpack.
            Default is disabled.
            \param flag erase database flag */
        void setEraseDatabase(bool flag);

        /** Configuration: set selected players.
            The selected players' files are those which are deleted by execute().
            Default is empty.
            \param set Player set */
        void setPlayers(PlayerSet_t set);

        /** Get selected players.
            \return player set as configured with setPlayers(). */
        PlayerSet_t getPlayers() const;

        /** Get remaining players.
            After scan() or execute(), returns the number of accessible players in this directory.
            \return remaining players */
        PlayerSet_t getRemainingPlayers() const;

     private:
        bool m_eraseDatabaseFlag;
        bool m_didScan;

        PlayerSet_t m_remainingPlayers;
        PlayerSet_t m_selectedPlayers;

        void processPlayerFiles(afl::io::Directory& dir, int player);
        void updateIndex(afl::io::Directory& dir);
    };

} }

#endif
