//
// Created by hataketsu on 12/24/18.
//

#ifndef ESPBOT_RGBFUNC_H
#define ESPBOT_RGBFUNC_H

#include "Arduino.h"
#include "GenericFuntion.h"
#include "../../../../../.platformio/packages/framework-arduinoespressif8266/cores/esp8266/WString.h"

class RGBFunc : GenericFuntion {
    String report() override {
        return color;
    }

    void click(int value) override {
        if (value == HIGH) {
            digitalWrite(this->r_pin, 0);
            digitalWrite(this->g_pin, 0);
            digitalWrite(this->b_pin, 0);
        } else {
            set_color(color);
        }
    }

public:
    explicit RGBFunc(int r, int g, int b) : GenericFuntion() {
        this->r_pin = r;
        this->g_pin = g;
        this->b_pin = b;
        pinMode(r_pin, OUTPUT);
        pinMode(g_pin, OUTPUT);
        pinMode(b_pin, OUTPUT);
        click(LOW);
        type = "rgb";
    }

    String color = "ffffff";

    void set_color(String color) {
        this->color = color;
        long rgb = strtol(&color[0], NULL, 16);
        Serial.println(rgb, 16);
        int red = rgb >> 16;

        int green = (rgb & 0x00ff00) >> 8;

        int blue = (rgb & 0x0000ff);

        analogWrite(this->r_pin, red * 1023 / 256);
        analogWrite(this->g_pin, green * 1023 / 256);
        analogWrite(this->b_pin, blue * 1023 / 256);
    }

    int r_pin;
    int g_pin;
    int b_pin;
};

#endif //ESPBOT_RGBFUNC_H
