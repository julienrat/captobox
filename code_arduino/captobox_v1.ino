//Import des bibliothèques ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>

//Import des bibliotheques SPIFFS
#include "FS.h"
#include "spiffs_lib.h"
#include "libs_capteurs.h"
//ticker pour update web data
#include <Ticker.h>
Ticker update_data;
Ticker AP_mode;
String capt1;
String capt2;
String frequency = "5000";
String client_mode;
String ssid_client;
String password_client;
String request;
String stream;

const char* host;

String buff[10]; // buffer de reception du fichier de config
boolean start_update = false;
int channel = 11; //- See more at: http://www.esp8266.com/viewtopic.php?f=23&t=4606#sthash.8jgsOvcY.dpuf

void tick_update()
{
  start_update = true;

}

void tick_AP(){
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);  
}

void setup() {
  Serial.begin(9600); //debug série

  // reset pin connected to rst
  pinMode(D0, OUTPUT);
  digitalWrite(D0, HIGH);
  //====
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);

  
  SPIFFS.begin(); // démarrage de la gestion SPIFFS
  read_file("/config.txt");
  capt1 = buff[1];
  capt2 = buff[2];
  frequency = buff[3];

  client_mode = buff[4];
  ssid_client = buff[5];
  password_client = buff[6];

  request = buff[7];
  host = buff[8].c_str();
  stream = buff[9];

  if (client_mode.equals("true")) {
    connection_wifi(ssid_client.c_str(), password_client.c_str()); // connexion au reseau wifi
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("captobox", "", 11);
    AP_mode.attach_ms(200, tick_update);    
  }




  serveur(); // serveur de fichiers
  server.begin(); // demarrage dur serveur de fichiers

  Serial.println("lecture de la configuration ");
  Serial.println("================================================ ");

  Serial.print("capt1 : ");
  Serial.println(capt1);
  Serial.print("capt2 : ");
  Serial.println(capt2);

  Serial.print("frequence : ");
  Serial.println(frequency);

  Serial.print("client  : ");
  Serial.println(client_mode);

  Serial.print("ssid : ");
  Serial.println(ssid_client);
  Serial.print("password : ");
  Serial.println(password_client);
  Serial.print("requete  : ");
  Serial.println(request);
  Serial.print("host : ");
  Serial.println(host);
  Serial.print("stream : ");
  Serial.println(stream);

  Serial.println("================================================ ");
  digitalWrite(BUILTIN_LED, HIGH);

  update_data.attach_ms(frequency.toInt(), tick_update);

}

void loop() {
  server.handleClient();

  if (stream.equals("true") && start_update) {
    Serial.print("tick : ");
    Serial.println(millis());
    send_json( host, request, String(sensor_start(capt1, 5)), String(sensor_start(capt2, RX)), String(sensor_start(capt1, 5)));
    Serial.print("tick : ");
    Serial.println(millis());
    start_update = false;
  }
}

void serveur() {




  server.on("/", HTTP_GET, []() {
    if (!handleFileRead("/index.html")) server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/config", HTTP_GET, []() {
    if (!handleFileRead("/config/config.html")) server.send(404, "text/plain", "FileNotFound");
  });

  server.on("/config/write", HTTP_GET, []() {
    String capt1 = server.arg("capt1");
    String capt2 = server.arg("capt2");
    String frequency = server.arg("frequency");
    String client_mode = server.arg("client");
    String ssid_client = server.arg("ssid");
    String password_client = server.arg("password");
    String request = server.arg("request");
    String host = server.arg("host");
    String stream = server.arg("stream");

    Serial.print("capteur 1 : ");
    Serial.println(capt1);
    Serial.print("capteur 2 : ");
    Serial.println(capt2);

    Serial.print("frequence : ");
    Serial.println(frequency);

    Serial.print("client : ");
    Serial.println(client_mode);
    Serial.print("ssid : ");
    Serial.println(ssid_client);
    Serial.print("password : ");
    Serial.println(password_client);
    Serial.print("requete : ");
    Serial.println(request);
    Serial.print("host : ");
    Serial.println(host);
    Serial.print("stream : ");
    Serial.println(stream);

    server.send(200, "text/plain", "okay");
    File configuration = SPIFFS.open("/config.txt", "w");
    configuration.print(capt1);
    configuration.print("$");
    configuration.print(capt2);
    configuration.print("$");

    configuration.print(frequency);
    configuration.print("$");

    configuration.print(client_mode);
    configuration.print("$");
    configuration.print(ssid_client);
    configuration.print("$");
    configuration.print(password_client);
    configuration.print("$");

    configuration.print(request);
    configuration.print("$");
    configuration.print(host);
    configuration.print("$");
    configuration.print(stream);
    configuration.print("$");
    server.send(200, "text/plain", "ecriture spiffs");
    configuration.close();
    Serial.println("fichier enregistré");
    delay(50);
    digitalWrite(D0, LOW); //reset de l'esp (D0 donnecté avec une resistance de 510 ohms à RST)

  });

  server.on("/lecture", HTTP_GET, []() {
    digitalWrite(BUILTIN_LED, LOW);
    String json = "{";
    json += "\"capt1\":\"" + String(sensor_start(capt1, 5)) + "\"";
    json += ", \"name_capt1\":\"" + String(sensor_name(capt1)) + "\"";
    json += ", \"min_capt1\":\"" + String(sensor_min(capt1)) + "\"";
    json += ", \"max_capt1\":\"" + String(sensor_max(capt1)) + "\"";
    json += ", \"capt2\":\"" + String(sensor_start(capt2, RX)) + "\"";
    json += ", \"name_capt2\":\"" + String(sensor_name(capt2)) + "\"";
    json += ", \"min_capt2\":\"" + String(sensor_min(capt2)) + "\"";
    json += ", \"max_capt2\":\"" + String(sensor_max(capt2)) + "\"";
    json += "}";
    server.send(200, "text/json", json);
    json = String();
    digitalWrite(BUILTIN_LED, HIGH);
  });
  server.on("/lecture_config", HTTP_GET, []() {
    // digitalWrite(BUILTIN_LED, LOW);
    String json = "{";
    json += "\"ssid_client\":\"" + ssid_client + "\"";
    json += ", \"password_client\":\"" + password_client + "\"";
    json += ", \"request\":\"" + request + "\"";
    json += ", \"client\":\"" + client_mode + "\"";
    json += ", \"frequency\":\"" + frequency + "\"";
    json += ", \"capt1\":\"" + capt1 + "\"";
    json += ", \"capt2\":\"" + capt2 + "\"";
    json += ", \"host\":\"" + String(host) + "\"";
    json += ", \"stream\":\"" + stream + "\"";
    json += "}";
    server.send(200, "text/json", json);

    json = String();
    // digitalWrite(BUILTIN_LED, HIGH);
  });


  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
}

void connection_wifi(const char* ssid, const char* password) {
  int timeout = 0;
  WiFi.begin(ssid, password);
  Serial.println("connection en cours...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    timeout++;
    Serial.print(".");
    if (timeout > 50) {
      File configuration = SPIFFS.open("/config.txt", "w");
      Serial.println("timeout");
      configuration.print(capt1);
      configuration.print("$");
      configuration.print(capt2);
      configuration.print("$");

      configuration.print(frequency);
      configuration.print("$");

      configuration.print("false");
      configuration.print("$");
      configuration.print(ssid_client);
      configuration.print("$");
      configuration.print(password_client);
      configuration.print("$");

      configuration.print(request);
      configuration.print("$");
      configuration.print(host);
      configuration.print("$");
      configuration.print(stream);
      configuration.print("$");
      configuration.close();
      delay(50);
      digitalWrite(D0, LOW); //reset de l'esp (D0 donnecté avec une resistance de 510 ohms à RST)
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}
void read_file(char* fichier) {
  File  file = SPIFFS.open(fichier, "r");
  if (!file) {
    Serial.println("file open failed");
  }  Serial.println("====== Lecture de la configuration dans le fichier SPIFFS  =======");
  // write 10 strings to file
  int i = 0;
  while (file.available()) {
    i++;
    buff[i] = file.readStringUntil('$');

  }

}

void send_json(const char* host_in, String url_in, String data1, String data2, String data3) {
  String contentType = "application/json";
  String json = "{";
  json += "\"value1\":\"" + data1 + "\"";
  json += ", \"value2\":\"" + data2 + "\"";
  json += "}";
  WiFiClient client;

  const int httpPort = 80;
  if (client.connect(host_in, httpPort)) {
    Serial.print("Requesting URL: ");
    Serial.println(url_in);

    // This will send the request to the server
    client.print(String("GET ") + url_in + " HTTP/1.1\r\n" +
                 "Host: " + host_in + "\r\n" +
                 "Cache-Control: no-cache" + "\r\n" +
                 "Content-Type: " + contentType + "\r\n" +
                 "Content-Length: " + json.length() + "\r\n" +
                 "\r\n" +
                 json + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

}
