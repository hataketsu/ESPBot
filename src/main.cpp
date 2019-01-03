#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include "function/GenericFuntion.h"
#include "function/OnOffFunc.h"
#include "function/DimmerFunc.h"
#include "function/RGBFunc.h"
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

static const char *const IN_TOPIC = "control/in";
static const char *const OUT_TOPIC = "control/out";
static const int BASE_EEPROM = 10;
static const int MAX_ROM = 400;
StaticJsonBuffer<2048> buffer;

const char *SERVER = "hungank.com";
WiFiClient espClient;
PubSubClient client(espClient);
WiFiManager wifiManager;

void onReceivingMessage(String string);

void report(String msg);

void reportJson(const JsonObject &data);

void save_names();

#define SMART_PLUG
//#define TEST_PROTOTYPE
//#define NIGHT_LAMP

#ifdef  TEST_PROTOTYPE
GenericFuntion *funtions[] = {
        (GenericFuntion *) new OnOffFunc(D2),
        (GenericFuntion *) new DimmerFunc(D3),
        (GenericFuntion *) new OnOffFunc(D4),
        (GenericFuntion *) new DimmerFunc(D5),
        (GenericFuntion *) new OnOffFunc(D6),};
#endif

#ifdef  SMART_PLUG
GenericFuntion *funtions[] = {
        (GenericFuntion *) new OnOffFunc(D1),
        (GenericFuntion *) new OnOffFunc(D2),
        (GenericFuntion *) new OnOffFunc(D5),
};
#endif

#ifdef  NIGHT_LAMP
GenericFuntion *funtions[] = {
        (GenericFuntion *) new RGBFunc(D3, D5, D6)
};
#endif


void callback(char *topic, byte *payload, unsigned int length) {
    payload[length] = 0;
    onReceivingMessage(String((char *) payload));
}

void report_func(GenericFuntion *func);

void onReceivingMessage(String msg) {
    buffer.clear();
    JsonObject &request = buffer.parseObject(msg);
    String command = request["command"];
    int value = request["value"];
    command.trim();
    if (command.equals("poll")) {
        for (GenericFuntion *func:funtions) {
            report_func(func);
        }
    } else if (command.equals("click")) {
        String id = request["id"];
        for (GenericFuntion *func:funtions) {
            if (func->UID.equals(id)) {
                func->click(value);
                report_func(func);
                break;
            }
        }
    } else if (command.equals("voice")) {
        String name = request["name"];
        for (GenericFuntion *func:funtions) {
            if (func->name.equals(name)) {
                func->click(value);
                report_func(func);
            }
        }
    } else if (command.equals("change_name")) {
        String id = request["id"];
        String name = request["name"];
        for (GenericFuntion *func:funtions) {
            if (func->UID.equals(id)) {
                func->name = name;
                save_names();
                report_func(func);
            }
        }
    } else if (command.equals("dim")) {
        String id = request["id"];
        int value = request["value"];
        for (GenericFuntion *func:funtions) {
            if (func->UID.equals(id)) {
                ((DimmerFunc *) func)->dimmer(value);
                report_func(func);
            }
        }
    } else if (command.equals("color")) {
        String id = request["id"];
        String name = request["name"];
        String color = request["color"];
        for (GenericFuntion *func:funtions) {
            if (func->UID.equals(id) || func->name.equals(name)) {
                ((RGBFunc *) func)->set_color(color);
                report_func(func);
            }
        }
    }
}

void report_func(GenericFuntion *func) {
    buffer.clear();
    JsonObject &data = buffer.createObject();
    data["id"] = func->UID;
    data["type"] = func->type;
    data["name"] = func->name;
    data["status"] = func->report();
    if (func->type.equals("dimmer"))
        data["value"] = ((DimmerFunc *) func)->value;
    reportJson(data);
}

void read_names();

void save_names() {
    StaticJsonBuffer<MAX_ROM> buffer;
    JsonArray &names = buffer.createArray();
    for (GenericFuntion *func:funtions) {
        names.add(func->UID);
        names.add(func->name);
    }
    String names_str = "";
    names.printTo(names_str);
    Serial.println(names_str);
    int i = 0;
    for (; i < names_str.length(); ++i) {
        EEPROM.put(BASE_EEPROM + i, names_str.c_str()[i]);
    }
    EEPROM.put(BASE_EEPROM + i, 0);
    EEPROM.commit();
}

void read_names() {
    StaticJsonBuffer<MAX_ROM> buffer;
    char c = -1;
    String names_str;
    for (int i = BASE_EEPROM; (i < MAX_ROM + BASE_EEPROM) && c != 0; ++i) {
        c = EEPROM.read(i);
        names_str.concat(c);
    }
    Serial.println(names_str);
    JsonArray &names = buffer.parseArray(names_str);
    for (int j = 0; j < sizeof(funtions) / sizeof(GenericFuntion *); ++j) {
        const char *id = names[j * 2];
        const char *name = names[j * 2 + 1];
        funtions[j]->UID = String(id);
        funtions[j]->name = String(name);
        Serial.println(name);
    }
}

void reportJson(const JsonObject &data) {
    String response;
    data.printTo(response);
    report(response);
}


void report(String msg) {
    client.publish(OUT_TOPIC, msg.c_str());
}


void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("connected");
            client.subscribe(IN_TOPIC);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 2 seconds");
            delay(2000);
        }
    }
}


void setup() {
    Serial.begin(9600);

    delay(1000);
    client.setServer(SERVER, 1883);
    client.setCallback(callback);
    EEPROM.begin(512);
//    save_names();
    read_names();
}

void loop() {

    if (!WiFi.isConnected()) {
        wifiManager.autoConnect("AutoConnectAP");
        ArduinoOTA.begin();
    }
    if (!client.connected()) {
        reconnect();
        for (GenericFuntion *func:funtions) {
            report_func(func);
        }
    }
    client.loop();
}