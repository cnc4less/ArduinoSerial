#include <SProtocol.h>

#define LED_PIN 13

SProtocol Host;

// Channel callback
void led13_channel(byte channel, byte *data, size_t size) {
    digitalWrite(LED_PIN, *data ? HIGH : LOW);
}


void setup() 
{                
    pinMode(LED_PIN, OUTPUT);

    Host.begin(115200);
    Host.send(0); // Notify the host machine

    Host.attach(1, led13_channel); // Attach channel callback
}


void loop() 
{
    // ...
}

void serialEventRun() {
    Host.processInput();
}