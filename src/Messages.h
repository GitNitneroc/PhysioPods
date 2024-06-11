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
        CONTROL,
        PING,
        ID_REORG,
        SSID
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

    struct SSIDMessage{
        uint16_t sessionId;
        uint64_t ssid;
    };

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

    struct PingMessage{ //the same struct is used for ping (client->server) and pong (server->client)
        uint8_t id; //always the id of the client (sender for ping, destination for pong)
        uint16_t sessionId;
    };

    struct IdReorgMessage{//this is used to reorganize the ids of a pod, switch him from old to newId
        uint8_t oldId;
        uint8_t newId;
        uint16_t sessionId;
        uint32_t space; //Note : this is a bit stupid, but makes us a different size of struct, for parsing purposes. Since this msg is not often used it's no big deal
    };
}
//constexpr size_t debugSize = sizeof(ControlMessage);

#endif //MESSAGES_H