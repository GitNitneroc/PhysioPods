#ifndef MESSAGES_H
#define MESSAGES_H

//This file is used to define the messages that are used to communicate between the serverpod and its clientpods

/*
    *The message that is used to send the state of the LED
    *id : the id of the clientpod
    *state : the state of the LED
*/
struct LEDMessage {
    int id;
    int state;
};

#endif //MESSAGES_H