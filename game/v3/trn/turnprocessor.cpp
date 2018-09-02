/**
  *  \file game/v3/trn/turnprocessor.cpp
  */

#include "game/v3/trn/turnprocessor.hpp"
#include "game/v3/inboxfile.hpp"

game::v3::trn::TurnProcessor::TurnProcessor()
{ }

game::v3::trn::TurnProcessor::~TurnProcessor()
{ }

void
game::v3::trn::TurnProcessor::handleTurnFile(TurnFile& f, afl::charset::Charset& charset)
{
    f.sortCommands();

    // Pass 1: verify commands
    for (size_t i = 0, n = f.getNumCommands(); i < n; ++i) {
        int cmdId;
        TurnFile::CommandCode_t cmdCode;
        TurnFile::CommandType cmdType;
        if (f.getCommandId(i, cmdId) && f.getCommandCode(i, cmdCode) && f.getCommandType(i, cmdType)) {
            switch (cmdType) {
             case TurnFile::UndefinedCommand:
                handleInvalidCommand(cmdCode);
                break;

             case TurnFile::ShipCommand:
                validateShip(cmdId);
                break;

             case TurnFile::PlanetCommand:
                validatePlanet(cmdId);
                break;

             case TurnFile::BaseCommand:
                validateBase(cmdId);
                break;

             case TurnFile::OtherCommand:
                break;
            }
        }
    }

    // Pass 2: process commands.
    String_t timAllies;
    size_t i = 0;
    while (i < f.getNumCommands()) {
        // Get this command's Id and class
        // (These calls will not fail; we have verified above.)
        int cmdId = 0;
        TurnFile::CommandType cmdType = TurnFile::UndefinedCommand;
        f.getCommandId(i, cmdId);
        f.getCommandType(i, cmdType);

        switch (cmdType) {
         case TurnFile::ShipCommand: {
            size_t n = f.findCommandRunLength(i);
            Ship_t rawData;
            getShipData(cmdId, rawData, charset);

            while (n > 0) {
                TurnFile::CommandCode_t cmdCode;
                int cmdLength;
                if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                    size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                    afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                    afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);

                    TurnFile::CommandCode_t nextCode;
                    if (cmdCode == tcm_ShipChangeFc && n >= 2 && f.getCommandCode(i+1, nextCode) && nextCode == tcm_ShipChangeFc && cmdData.size() >= 3) {
                        // might be THost alliance command
                        char out[3];
                        afl::base::fromObject(out).copyFrom(cmdData);
                        if ((out[0] == 'f' && out[1] == 'f') || (out[0] == 'F' && out[1] == 'F') || (out[0] == 'e' && out[1] == 'e')) {
                            timAllies.append(out, sizeof(out));
                        }
                    }
                }
                ++i, --n;
            }

            storeShipData(cmdId, rawData, charset);
            break;
         }

         case TurnFile::PlanetCommand: {
            size_t n = f.findCommandRunLength(i);
            Planet_t rawData;
            getPlanetData(cmdId, rawData, charset);

            while (n > 0) {
                TurnFile::CommandCode_t cmdCode;
                int cmdLength;
                if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                    if (cmdCode == tcm_PlanetBuildBase) {
                        rawData.buildBaseFlag = 1;
                    } else {
                        size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                        afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                        afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);
                    }
                }
                ++i, --n;
            }

            storePlanetData(cmdId, rawData, charset);
            break;
         }

         case TurnFile::BaseCommand: {
            size_t n = f.findCommandRunLength(i);
            Base_t rawData;
            getBaseData(cmdId, rawData, charset);

            while (n > 0) {
                TurnFile::CommandCode_t cmdCode;
                int cmdLength;
                if (f.getCommandCode(i, cmdCode) && f.getCommandLength(i, cmdLength)) {
                    size_t offset = TurnFile::getCommandCodeRecordIndex(cmdCode);
                    afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                    afl::base::fromObject(rawData).subrange(offset, cmdLength).copyFrom(cmdData);
                }
                ++i, --n;
            }

            storeBaseData(cmdId, rawData, charset);
            break;
         }

         case TurnFile::OtherCommand: {
            TurnFile::CommandCode_t cmdCode = 0;
            f.getCommandCode(i, cmdCode);
            if (cmdCode == tcm_SendMessage) {
                afl::base::ConstBytes_t cmdData = f.getCommandData(i);
                /* format of message is:
                      id     = length
                      data+0 = sender
                      data+2 = receiver
                      data+4 = text */
                structures::Int16_t to;
                to = 0;
                afl::base::fromObject(to).copyFrom(cmdData.subrange(2, 2));
                int size = cmdId;
                addMessage(to, decodeMessage(cmdData.subrange(4, size), charset, false));
            } else if (cmdCode == tcm_ChangePassword) {
                NewPassword_t pass;
                afl::base::Bytes_t(pass).copyFrom(f.getCommandData(i));
                addNewPassword(pass);
            } else {
                // unknown
            }
            ++i;
            break;
         }

         case TurnFile::UndefinedCommand:
            // Cannot happen, but avoid lossage if it does anyway
            ++i;
            break;
        }
    }

    if (!timAllies.empty()) {
        addAllianceCommand(charset.decode(afl::string::toBytes(timAllies)));
    }
}
