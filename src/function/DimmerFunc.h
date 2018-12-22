//
// Created by hataketsu on 12/16/18.
//

#ifndef ESPBOT_DIMMERFUNC_H
#define ESPBOT_DIMMERFUNC_H


#include <WString.h>
#include "GenericFuntion.h"

class DimmerFunc : GenericFuntion {
    String report() override {
        String tmp = String(value);
        tmp.concat(" %");
        return tmp;
    }

    void click(int value) override {
        dimmer((1 - value) * 100);
    }

public:
    explicit DimmerFunc(int _pin) : GenericFuntion() {
        pin = _pin;
        pinMode(pin, OUTPUT);
        dimmer(0);
        type = "dimmer";
    }

    int pin;
    int value;

    void dimmer(int value) {
        value = max(min(value, 100), 0);
        analogWrite(pin, (100 - value) * 1023 / 100);
        this->value = value;
    }
};


#endif //ESPBOT_DIMMERFUNC_H
