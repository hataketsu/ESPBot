//
// Created by hataketsu on 12/16/18.
//

#ifndef ESPBOT_ONOFFFUNC_H
#define ESPBOT_ONOFFFUNC_H

#include "Arduino.h"
#include "GenericFuntion.h"

class OnOffFunc : GenericFuntion {
    String report() override {
        return digitalRead(pin) == HIGH ? "tắt" : "bật";
    }

public:
    int pin;

    void click(int value) override {
        digitalWrite(this->pin, value);
    }

    explicit OnOffFunc(int _pin) : GenericFuntion() {
        pin = _pin;
        pinMode(pin, OUTPUT);
        digitalWrite(pin, HIGH);
        type = "onoff";
    }
};

#endif //ESPBOT_ONOFFFUNC_H
