#ifndef MESSAGES_H
#define MESSAGES_H
#include <Arduino.h>

//This file is used to define the messages that are used to communicate between the serverpod and its clientpods
//they are parsed based on their length and then casted to the appropriate struct
namespace Messages{

    enum MessageType {
        ERROR,
        NOT_FOR_ME,
        LED,
        CONTROL
    };

    /*
        *This struct is used to parse the message received from the clientpod
        *type : the type of the message
        *messageData : the data of the message
    */
    struct parsedMessage {
        MessageType type;
        void* messageData;
    };

    /*
        *This function is used to parse the message received from the clientpod
        *sender_addr : the address of the sender
        *data : the data received
        *len : the length of the data
    */
    parsedMessage getMessageType(const uint8_t * sender_addr, const uint8_t *data, int len);

    /*
        *The message that is used to send the state of the LED
        *id : the id of the clientpod
        *sessionId : the session id of the pods
        *state : the state of the LED
        *mode : the mode of the LED, for different light patterns
        *r : the red value of the LED
        *g : the green value of the LED
        *b : the blue value of the LED
    */
    struct LEDMessage {
        uint8_t id;
        uint16_t sessionId;
        bool state;
        uint8_t mode;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    /*
        *The message that is used to send the state of the control
        *id : the id of the clientpod
        *sessionId : the session id of the pods
        *state : the state of the control
    */
    struct ControlMessage {
        uint8_t id;
        uint16_t sessionId;
        bool state; //in case we need to send a state, but for now it's only sent once on the press
    };
}
//constexpr size_t debugSize = sizeof(ControlMessage);

#endif //MESSAGES_H