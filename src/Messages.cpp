#include "Messages.h"
#include "PhysioPod.h"
using namespace Messages;

parsedMessage Messages::getMessageType(const uint8_t * sender_addr, const uint8_t *data, int len){
    parsedMessage message;
    //the first byte of the message is the type
    MessageType type = (MessageType)data[0];
    switch (type){
        case MessageType::PING:{
            message.type = MessageType::PING;
            Serial.println("Ping message received");
            PingMessage* pingMessage = (PingMessage*)data;
            //check sessiongId
            if (pingMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            message.messageData = pingMessage;
            break;
        }
        case MessageType::CONTROL:{
            Serial.println("Control message received");
            message.type = MessageType::CONTROL;
            ControlMessage* controlMessage = (ControlMessage*)data;
            //check sessionId
            if (controlMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            //check message Id
            if (controlMessage->id != PhysioPod::getInstance()->getId() && controlMessage->id != 255){
                message.type = MessageType::NOT_FOR_ME;
            }
            message.messageData = controlMessage;
            break;
        }
        case MessageType::ID_REORG:{
            Serial.println("IdReorg message received");
            message.type = MessageType::ID_REORG;
            IdReorgMessage* reorgMessage = (IdReorgMessage*)data;
            //check sessiongId
            if (reorgMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            //check oldId
            if (reorgMessage->oldId != PhysioPod::getInstance()->getId()){
                message.type = MessageType::NOT_FOR_ME;
            }
            message.messageData = reorgMessage;
            break;
        }
        
        case MessageType::LED:{
            Serial.println("LED message received");
            message.type = MessageType::LED;
            LEDMessage* ledMessage = (LEDMessage*)data;
            //check sessionId
            if (ledMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            //check message Id
            if (ledMessage->id != PhysioPod::getInstance()->getId() && ledMessage->id != 255){
                message.type = MessageType::NOT_FOR_ME;
            }
            message.messageData = ledMessage;
            break;
        }
        case MessageType::SSID:{
            message.type = MessageType::SSID;
            Serial.println("SSID message received");
            SSIDMessage* ssidMessage = (SSIDMessage*)data;
            //check sessionId
            if (ssidMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            //all SSID messages are for everyone, so no need to check the id
            message.messageData = ssidMessage;
            break;
        }
        default:
            Serial.println("Unknown message received");
            message.type = MessageType::ERROR;
            break;
    }
    return message;
}