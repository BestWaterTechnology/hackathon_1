
#include <WiFiClientSecure.h>
//#include <MQTT.h>
#include <PubSubClient.h>
#include <WebServer.h>

const char ssid[] = "";
const char pass[] = "";

int blinky = 0;
int blinky_count = 0;

int blinky2 = 0;
int blinky2_count = 0;

//const char* mqttServer = "192.168.0.16";
const char* mqttServer = "rnddevmqttvpn.westeurope.azurecontainer.io";

const int mqttPort = 1883;

const char* mqttUser = "device";
const char* mqttPassword = "dev1234";

// PRe-declaration
void callback(char* topic, byte* payload, unsigned int length);

WiFiClient net;

PubSubClient clientPub(net);

// Port 80 host
WebServer server(80);

unsigned long lastMillis = 0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  

  Serial.print("\nconnecting...");

   while (!clientPub.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (clientPub.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(clientPub.state());
      delay(2000);
 
    }
  }
  
}

void handle_get() {
  String content = "<HTML>";
  content += "<BODY><H1>Hello, world</H1></BODY>";
  content += "</HTML>";

  server.send(200, "text/html", content);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.
  //
  // MQTT brokers usually use port 8883 for secure connections.
  //client.begin("192.168.0.16", 1883, net);
  //client.onMessage(messageReceived);

  clientPub.setServer(mqttServer, mqttPort);
  clientPub.setCallback(callback);

  pinMode(22, OUTPUT);
  pinMode(23, OUTPUT);

  connect();
  Serial.print("Subscribing\n");
  if(clientPub.subscribe("24-6F-28-AB-2D-54/command")) {
    Serial.print("Subscribing OK\n");
  }

  clientPub.publish("/hello", "Started up");

  clientPub.publish("/hello/ip", WiFi.localIP().toString().c_str());

  server.on("/", handle_get);
  server.begin();

}

void post() {
      char str[32];
    sprintf(str,"%d", blinky2_count);
    String topic = WiFi.macAddress();
    topic.replace(':','-');

    topic += "/blinky2/count";
    clientPub.publish(topic.c_str(), str);

    sprintf(str,"%d", blinky2);
    topic = WiFi.macAddress();
    topic.replace(':','-');
    topic += "/blinky2/status";
    clientPub.publish(topic.c_str(), str);

}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  if( payload[0] == '0' ) {
    blinky2 = 0;
  } else {
    blinky2 = 1;
  }
  

  digitalWrite(23, (blinky2)?HIGH:LOW);
  blinky2_count++;


  post();

  Serial.println();
  Serial.println("-----------------------");
 
}



void loop() {
  clientPub.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
 //   if (!client.connected()) {
 //     Serial.print("lastError: ");
 //     Serial.println(client.lastError());
 //     connect();

   Serial.print("Send...\n");
    clientPub.publish("/hello", "world from oxford");
    lastMillis = millis();
    digitalWrite(22, (blinky ^= 1)?HIGH:LOW);
    blinky_count ++;
     char str[32];
    sprintf(str,"%d", blinky_count);

    
    String topic = WiFi.macAddress();
    topic.replace(':','-');

    topic += "/blinky/count";
    clientPub.publish(topic.c_str(), str);

    sprintf(str,"%d", blinky);
    topic = WiFi.macAddress();
    topic.replace(':','-');
    topic += "/blinky/status";
    clientPub.publish(topic.c_str(), str);

    }
    
   server.handleClient();
}
