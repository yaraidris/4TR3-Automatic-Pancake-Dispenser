#include "cloud.h"
void checkClient(){
  client.loop();
}

bool Connect()
{
  // Connect to MQTT Server and subscribe to the topic
  if (client.connect(clientID, mqtt_username, mqtt_password))
  {
    client.subscribe(mqtt_topic);
    return true;
  }
  else
  {
    return false;
  }
}

static uint16_t getMeasurement()
{
  VL53L0X_RangingMeasurementData_t measure;

  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // false = no debug

  if (measure.RangeStatus != 4)
  {
    Serial.print("Distance (mm): ");
    Serial.println(measure.RangeMilliMeter);
    return measure.RangeMilliMeter;
  }
  else
  {
    Serial.println(" out of range ");
    return 0;
  }
}

//converts the level read into a precent of total level
static uint16_t getBatterLevel()
{
  if (getMeasurement()>totalDistance){
    return 0;
  }
  return (uint16_t)(totalDistance - getMeasurement());
}


void cloudReconnect() {
  //While the client remains unconnected from the MQTT broker, attempt to reconnect every 2 seconds
  //Also, print diagnostic information
  while (!client.connected()) {
    Serial.print("\nAttempting MQTT connection...");
  
    if (client.connect("ESP8266Client")) {
      Serial.println("Connected to MQTT server");  
      client.subscribe("testTopic");
    } else {
      Serial.print("\nFailed to connect to MQTT server, rc = ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

//Initializes ESP module and sensor
void Cloud_init()
{
  // initialize ESP module
  WiFi.init(&Serial1);
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true)
      ;
  }
  // attempt to connect to WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // Once connected, print the local IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  while (!Serial)
  {
    delay(1);
  }

// Check if ToF is connected and working
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin())
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1)
      ;
  }

  // Connect to MQTT Broker

  client.setServer(brokerAddress, addressPort);
  if (Connect())
  {
    Serial.println("Connected Successfully to MQTT Broker!");
  }
  else
  {
    Serial.println("Connection Failed!");
  }

}

//Publishes level data to MQTT broker
void Cloud_sendData()
{
  uint16_t batterLevel = getBatterLevel();
  float batterLevelPercent = (batterLevel / totalDistance) * 100;
  Serial.print("Level Percent: ");
  Serial.println(batterLevelPercent);

  if (!client.connected())
  {
    cloudReconnect(); 
  }

  // 3. Publish only if connected
  if (client.connected())
  {
    client.publish("Capstone/BatterLevel", String(batterLevelPercent).c_str()); 
    Serial.println("Published data.");
  }

  
}
