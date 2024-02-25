#include <Arduino.h>
#include "isDebug.h"
#include "scoreStorage.h"

ScoreStorage::ScoreStorage()
{
    //init scores with NULL
    for (uint8_t i=0; i<3; i++){
        scores[i] = NULL;
    }
}

ScoreStorage::~ScoreStorage()
{
    //it doesn't seem very useful to delete the scores, but let's do it anyway
    for (uint8_t i=0; i<3; i++){
        if (scores[i] != NULL){
            delete scores[i];
        }
    }
}

/*
    * Call this to update the current score
 */
void ScoreStorage::updateScore(String* score){
    if (scores[0] != NULL){
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
        if (scores[i] != NULL){
            json += *scores[i];
        }
        //add a comma if it's not the last score, and if it's not empty
        if (i<2 && scores[i+1] != NULL){
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
    if (scores[0] != NULL){
        return *scores[0];
    }
    return "";
}