//
// Created by hataketsu on 12/16/18.
//

#ifndef ESPBOT_GENERICFUNTION_H
#define ESPBOT_GENERICFUNTION_H

#include "Arduino.h"

class GenericFuntion {
public:
    String genUID() {
        String buf = "";
        for (int i = 0; i < 16; ++i) {
            buf.concat((char) random('a', 'z'));
        }
        return buf;
    }

    virtual void click(int value) =0;

    void dimmer(int value) {};

    GenericFuntion() {
        UID = this->genUID();
        name = UID;
    }

    virtual String report()= 0;

    String name;

    String UID;
    String type;
};


#endif //ESPBOT_GENERICFUNTION_H
