#include <ESP8266WiFi.h>
#include <PubSubClient.h>


static const char *const IN_TOPIC = "hataketsucontrol/in";
static const char *const OUT_TOPIC = "hataketsucontrol/out";
const char *ssid = "Franxx";

const char *password = "nopenope";
const char *mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);

void onReceivingMessage(String string);

void sendBack();

int PINS[] = {D0, D1, D2, D3, D4, D5, D6, D7, D8};

void setup_wifi() {

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = 0;
    onReceivingMessage(String((char *) payload));
}

void onReceivingMessage(String msg) {

    int index = msg.indexOf(':');
    String command = msg.substring(0, index);
    String value = msg.substring(index + 1, msg.length());
    if (command.equals("poll")) {
        sendBack();
    } else {
        int cmd = command.toInt();
        int val = value.toInt();

        Serial.println(cmd);
        Serial.println(val);
        digitalWrite(PINS[val], cmd);
        sendBack();
    }
}

void sendBack() {
    String buf = "";
    for (int pin :PINS) {
        buf.concat(digitalRead(pin));
    }
    client.publish(OUT_TOPIC,buf.c_str());
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

    for (int pin:PINS) {
        pinMode(pin, OUTPUT);
    }
    Serial.begin(9600);
    delay(1000);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
    while (WiFi.status() != WL_CONNECTED)
        setup_wifi();
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}