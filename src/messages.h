#ifndef MESSAGES_H
#define MESSAGES_H

//This file is used to define the messages that are used to communicate between the serverpod and its clientpods

/*
    *The message that is used to send the state of the LED
    *id : the id of the clientpod
    *sessionId : the session id of the pods
    *state : the state of the LED
    *mode : the mode of the LED, for different light patterns
*/
struct LEDMessage {
    uint8_t id;
    uint16_t sessionId;
    bool state;
    uint8_t mode;
};

struct ControlMessage {
    uint8_t id;
    uint16_t sessionId;
    bool state; //in case we need to send a state, but for now it's only sent once on the press
};

//constexpr size_t debugSize = sizeof(LEDMessage);

#endif //MESSAGES_H