#include <Arduino.h>
#include "isDebug.h"
#include "scoreStorage.h"

String* scores[3];

void ScoreStorage::init()
{
    //init scores with NULL
    for (uint8_t i=0; i<3; i++){
        scores[i] = nullptr;
    }
}

/*
    * Call this to update the current score
 */
void ScoreStorage::updateScore(String* score){
    if (scores[0] != nullptr){
        delete scores[0];
    }
    scores[0] = score;
}

/*
    * Call this to add a new score to the list
    * create this new score even if it's empty, it will be updated later
    * The oldest score will be deleted
 */
void ScoreStorage::addScore(String* score){
    #ifdef isDebug
    Serial.print("Adding this score to storage : ");
    Serial.println(*score);
    //Serial.println(sizeof(score));
    //print the score memory addresses
    for (uint8_t i=0; i<3; i++){
        Serial.printf("Score %d : %p\n", i, scores[i]);
    }
    #endif

    delete scores[2];
    scores[2] = scores[1];
    scores[1] = scores[0];
    scores[0] = score;
}

/*
    * Call this to get the scores in JSON format
 */
String ScoreStorage::getAllJSON(){
    String json = "[";
    for (uint8_t i=0; i<3; i++){
        if (scores[i] != nullptr){
            json += *scores[i];
        }
        //add a comma if it's not the last score, and if it's not empty
        if (i<2 && scores[i+1] != nullptr){
            json += ",";
        }
    }
    json += "]";
    return json;
}

/*
    * Call this to get the last score in JSON format
    * returns an empty string if there is no score
 */
String ScoreStorage::getLastJSON(){
    if (scores[0] != nullptr){
        return *scores[0];
    }
    return "";
}