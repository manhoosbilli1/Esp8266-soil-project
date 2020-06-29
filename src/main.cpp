#include <JC_Button.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#define FIREBASE_HOST "led1-b7e49.firebaseio.com"
#define FIREBASE_AUTH "rQjkcrr0KciU1Opt9CaKlPQto0byD2qZNqPyUuvi"
#define WIFI_SSID "Shoaib" //your WiFi SSID for which yout NodeMCU connects
#define WIFI_PASSWORD "A0123456789"

int val1;
int last_val;
Button mybtn(4);
int relay = 13;
void setup()
{
    mybtn.begin();
    Serial.begin(9600);
    pinMode(relay, OUTPUT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected:");
    Serial.println(WiFi.localIP());
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.setInt("light1",1); //Here the varialbe"S1","S2","S3" and "S4" needs to be the one which is used in our Firebase and MIT App Inventor
    digitalWrite(relay, LOW);
}

void loop()
{
    mybtn.read();
    val1 = Firebase.getString("S1").toInt();
    last_val;
    /* if (mybtn.wasReleased() || val1 == 1)
  {
    digitalWrite(relay, !digitalRead(relay));
    Firebase.set("light1", 1);
    val1 = !val1;
    Serial.println(val1);
  }
    */
    if (val1 == 1)
    {
        digitalWrite(relay, HIGH);
    }
}