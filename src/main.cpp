#include "Arduino.h"
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Set these to run example.
#define FIREBASE_HOST "soil-agriculture-iot.firebaseio.com"
#define FIREBASE_AUTH "yfWcnVTrxz5uPjx4arkx1auBxiEa2vL6yapPHJa0"
#define WIFI_SSID "Shoaib"
#define WIFI_PASSWORD "A0123456789"

//geolocation setttings
const char *Host = "www.unwiredlabs.com";
String endpoint = "/v2/process.php";
String token = "03c08ce47142d8";
String jsonString = "{\n";
double latitude = 0.0;
double longitude = 0.0;
double accuracy = 0.0;
float balance = 0.0;
bool run = true;
int error;
volatile bool startReading = false;
//----
String data = "";
SoftwareSerial Ser;
String ph, gas, moisture, humidity;
float n = 0.0;
int tries = 0;
bool valid = false;

int decode(String c)
{
  String phL = c.substring(c.indexOf('*') + 1, c.indexOf(','));
  String moistureL = c.substring(c.indexOf(',') + 1, c.indexOf('!'));
  String humidityL = c.substring(c.indexOf('!') + 1, c.indexOf('@'));
  String gasL = c.substring(c.indexOf('@') + 1, c.indexOf('?'));
  Serial.print(ph);
  Serial.print(',');
  Serial.print(moisture);
  Serial.print(',');
  Serial.print(humidity);
  Serial.print(',');
  Serial.println(gas);
  if (phL == NULL || moistureL == NULL || humidityL == NULL || gasL== NULL)
  {
    Ser.flush();
    Serial.println("Data missing, returning");
    return 1; //some data is missing
  }
  else
  {
    ph = phL;
    moisture = moistureL;
    gas = gasL;
    humidity = humidityL;
    Serial.println("Succesfully received data");
    Ser.flush();
    return 0;
  }
}

void updateFirebase()
{

  Firebase.pushFloat("History/gas", gas.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update history");
    delay(500);
    return;
  }
  Firebase.pushFloat("History/moisture", moisture.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update history");
    delay(500);
    return;
  }
  Firebase.pushFloat("History/ph", ph.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update history");
    delay(500);
    return;
  }
  Firebase.pushFloat("History/humidity", humidity.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update history");
    delay(500);
    return;
  }

  //updating individual live values for main screen
  Firebase.setFloat("firebaseIOT/gas", gas.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update live data");
    delay(500);
    return;
  }
  Firebase.setFloat("firebaseIOT/moisture", moisture.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update live data");
    delay(500);
    return;
  }
  Firebase.setFloat("firebaseIOT/ph", ph.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update live data");
    delay(500);
    return;
  }
  Firebase.setFloat("firebaseIOT/humidity", humidity.toFloat());
  if (Firebase.failed())
  {
    Serial.println("Failed to update live data");
    delay(500);
    return;
  }
  //update the live data feed firebaseIOT by setting
  //update the history by pushing data
  Serial.println("Updated database.");
}

void fetchCoordinates()
{
  while (latitude == 0.0 && tries < 10)
  {

    char bssid[6];
    DynamicJsonBuffer jsonBuffer;

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");

    if (n == 0)
    {
      Serial.println("No networks available");
    }
    else
    {
      Serial.print(n);
      Serial.println(" networks found");
    }

    // now build the jsonString...
    jsonString = "{\n";
    jsonString += "\"token\" : \"";
    jsonString += token;
    jsonString += "\",\n";
    jsonString += "\"id\" : \"saikirandevice01\",\n";
    jsonString += "\"wifi\": [\n";
    for (int j = 0; j < n; ++j)
    {
      jsonString += "{\n";
      jsonString += "\"bssid\" : \"";
      jsonString += (WiFi.BSSIDstr(j));
      jsonString += "\",\n";
      jsonString += "\"signal\": ";
      jsonString += WiFi.RSSI(j);
      jsonString += "\n";
      if (j < n - 1)
      {
        jsonString += "},\n";
      }
      else
      {
        jsonString += "}\n";
      }
    }
    jsonString += ("]\n");
    jsonString += ("}\n");
    Serial.println(jsonString);

    WiFiClientSecure client;
    client.setInsecure();
    //Connect to the client and make the api call
    Serial.println("Requesting URL: https://" + (String)Host + endpoint);
    if (client.connect(Host, 443))
    {
      Serial.println("Connected");
      client.println("POST " + endpoint + " HTTP/1.1");
      client.println("Host: " + (String)Host);
      client.println("Connection: close");
      client.println("Content-Type: application/json");
      client.println("User-Agent: Arduino/1.0");
      client.print("Content-Length: ");
      client.println(jsonString.length());
      client.println();
      client.print(jsonString);
      delay(500);
    }

    //Read and parse all the lines of the reply from server
    while (client.available())
    {
      String line = client.readStringUntil('\r');
      JsonObject &root = jsonBuffer.parseObject(line);
      if (root.success())
      {
        latitude = root["lat"];
        longitude = root["lon"];
        accuracy = root["accuracy"];
        balance = root["balance"];

        Serial.println();
        Serial.print("Latitude = ");
        Serial.println(latitude, 6);
        Serial.print("Longitude = ");
        Serial.println(longitude, 6);
        Serial.print("Accuracy = ");
        Serial.println(accuracy);
        Serial.print("Balance Left= ");
        Serial.println(balance);
      }
    }

    Serial.println("closing connection");
    Serial.println();
    client.stop();

    delay(5000);
    tries++;
  }
  if (latitude == 0.0 && tries == 10)
  {
    Serial.println("Tried to fetch coordinates but failed, check balance or connection");
  }
  digitalWrite(LED_BUILTIN, LOW);
  Firebase.setFloat("location/balance", balance);
  Firebase.setFloat("location/lat", latitude);
  Firebase.setFloat("location/longitude", longitude);
}

void checkData()
{
  while(Ser.available() > 0)
  {
    data = Ser.readStringUntil('\n');
  }
  if(data != NULL)
  {
    error = decode(data);
    if(error == 0){
    updateFirebase();
    }else {
      Serial.println("Data was currupted, not updating database");
    }
    data = "";
  }
}

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Ser.begin(9600, SWSERIAL_8N1, D5);
  Serial.begin(9600);
  digitalWrite(LED_BUILTIN, HIGH);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("Setup done");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Connected to firebase");
  fetchCoordinates();
}

void loop()
{
  checkData();
}

/*
1- Requests limit of unwired is 100 and so have to find multiple ways to get location. 
*/