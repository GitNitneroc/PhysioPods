#include "Messages.h"
#include "PhysioPod.h"
using namespace Messages;

parsedMessage Messages::getMessageType(const uint8_t * sender_addr, const uint8_t *data, int len){
    parsedMessage message;
    switch (len){
        case sizeof(PingMessage):{
            message.type = MessageType::PING;
            PingMessage* pingMessage = (PingMessage*)data;
            //check sessiongId
            if (pingMessage->sessionId != PhysioPod::getInstance()->getSessionId()){
                message.type = MessageType::ERROR;
            }
            message.messageData = pingMessage;
            break;
        }
        case sizeof(ControlMessage):{
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
        case sizeof(LEDMessage):{
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
        default:
            message.type = MessageType::ERROR;
            break;
    }
    return message;
}