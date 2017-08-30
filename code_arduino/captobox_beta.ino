//Import des bibliothèques ESP8266
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <Time.h>
#include <elapsedMillis.h>
#include "OneButton.h"
//Import des bibliotheques SPIFFS
#include "FS.h"
#include "spiffs_lib.h"
#include "libs_capteurs.h"
String version="version beta +++";

const int pin_sensor1 = RX;
const int pin_sensor2 = 5;
String capt1;
String capt2;
String frequency = "5000";
String client_mode;
String ssid_client;
String password_client;
String request;
String stream;
String save = "false";
boolean is_saving = false;
boolean write_title = true;
const char* host;

String buff[10]; // buffer de reception du fichier de config
boolean start_update = false;
int channel = 11; //- See more at: http://www.esp8266.com/viewtopic.php?f=23&t=4606#sthash.8jgsOvcY.dpuf




OneButton button(D3, true);

elapsedMillis timeElapsed;
unsigned int interval = 1000;

void setup() {
  delay(1000);
  Serial.begin(9600); //debug série

  button.attachDuringLongPress(erase_file);
  button.attachDoubleClick(save_on);
  // reset pin connected to rst
  pinMode(D0, OUTPUT);

  digitalWrite(D0, LOW);
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
    WiFi.mode(WIFI_STA);
    connection_wifi(ssid_client.c_str(), password_client.c_str()); // connexion au reseau wifi
    web_update();


  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("captobox", "", 2);

  }
  MDNS.begin("captobox");
  MDNS.addService("http", "tcp", 80);



  serveur(); // serveur de fichiers

  server.begin(); // demarrage dur serveur de fichiers



  Serial.println("");
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



}

void loop() {
  server.handleClient();
  button.tick();

  if (timeElapsed > frequency.toInt())
  {
    if (stream.equals("true")) {
      Serial.print("tick : ");
      Serial.println(millis());
      send_json( host, request, String(sensor_start(capt1, pin_sensor1)), String(sensor_start(capt2, pin_sensor2)), String(sensor_start(capt1, pin_sensor1)));
      Serial.print("tick : ");
      Serial.println(millis());
    }
    savedata();
    timeElapsed = 0;
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
    capt1 = server.arg("capt1");
    capt2 = server.arg("capt2");
    frequency = server.arg("frequency");
    client_mode = server.arg("client");
    ssid_client = server.arg("ssid");
    password_client = server.arg("password");
    request = server.arg("request");
    //host = server.arg("host");
    stream = server.arg("stream");

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
    delay(500);
    digitalWrite(D0, LOW);
    //ESP.restart();
    delay(500);
    //     //reset de l'esp (D0 donnecté avec une resistance de 510 ohms à RST)

  });
  server.on("/capt1_raw", HTTP_GET, []() {

    float val_capt1 = sensor_start(capt1, pin_sensor1);
    server.send(200, "text/json", String(val_capt1));


  });

  server.on("/capt2_raw", HTTP_GET, []() {

    float val_capt2 = sensor_start(capt2, pin_sensor2);
    server.send(200, "text/json", String(val_capt2));


  });
  server.on("/lecture", HTTP_GET, []() {


    String json = "{";
    json += "\"capt1\":\"" + String(sensor_start(capt1, pin_sensor1)) + "\"";
    json += ", \"name_capt1\":\"" + String(sensor_name(capt1)) + "\"";
    json += ", \"min_capt1\":\"" + String(sensor_min(capt1)) + "\"";
    json += ", \"max_capt1\":\"" + String(sensor_max(capt1)) + "\"";
    //delay(50);
    json += ", \"capt2\":\"" + String(sensor_start(capt2, pin_sensor2)) + "\"";
    json += ", \"name_capt2\":\"" + String(sensor_name(capt2)) + "\"";
    json += ", \"min_capt2\":\"" + String(sensor_min(capt2)) + "\"";
    json += ", \"max_capt2\":\"" + String(sensor_max(capt2)) + "\"";
    json += ", \"save\":\"" + String(save) + "\"";
    json += "}";
    server.send(200, "text/json", json);
    json = String();



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
    json += ", \"save\":\"" + String(save) + "\"";
    json += ", \"vers\":\"" + version+ "\"";
   
    json += "}";
    server.send(200, "text/json", json);

    json = String();
    // digitalWrite(BUILTIN_LED, HIGH);
  });

  //  == upload firmware

  server.onFileUpload([]() {

    HTTPUpload& upload = server.upload();
    String filename;
    if (server.uri() != "/update") return;

    if (upload.status == UPLOAD_FILE_START) {
      filename = upload.filename;
      Serial.print("Upload Name: "); Serial.println(filename);
      if (upload.filename.endsWith(".spiffs.bin")) {
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

        Serial.println("update to spiffs");
        if (!Update.begin(maxSketchSpace, U_SPIFFS)) { //start with max available size
          Update.printError(Serial);
          Serial.println("ERROR");
        }

      }
      if (upload.filename.endsWith(".d1_mini.bin")) {
        Serial.println("update to flash");
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(Serial);
          Serial.println("ERROR");
        }
      }

    } else if (upload.status == UPLOAD_FILE_WRITE) {
      Update.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
      Update.end(true);
    }

  });



  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK rebootez la captobox");
    delay(50);
    digitalWrite(D0, LOW); //reset de l'esp (D0 donnecté avec une resistance de 510 ohms à RST)

  });
  // == == == == == == == == ==
  server.on("/save_spiff", HTTP_GET, []() {
    server.send(200, "text/plain", "save ok");
    Serial.println("commande sauvegarde OK");
    save = "true";
  });

  server.on("/stop_save_spiff", HTTP_GET, []() {
    server.send(200, "text/plain", "save stop");
    save = "false";
  });

  server.on("/erase_save_spiff", HTTP_GET, []() {

  });
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
}
void savedata() {

  if (save.equals("true")) {
    if (write_title) {

      File save_file = SPIFFS.open("/save.csv", "a");
      save_file.print("date");
      save_file.print(":");
      save_file.print(sensor_name(capt1));
      save_file.print(":");
      save_file.print(sensor_name(capt2));
      save_file.print("\n");
      save_file.close();
      write_title = false;
    }
    digitalWrite(BUILTIN_LED, LOW);
    String val_capt1 = String(sensor_start(capt1, pin_sensor1));
    String val_capt2 = String(sensor_start(capt2, pin_sensor2));

    //ici on sauve

    File save_file = SPIFFS.open("/save.csv", "a");
    save_file.print(String(year()) + "," + String(month()) + "," + String(day()) + "," + String(hour()) + "," + String(minute()) + "," + String(second()));
    save_file.print(":");
    save_file.print( val_capt1);
    save_file.print(":");
    save_file.print(val_capt2);
    save_file.print("\n");
    save_file.close();
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void erase_file() {
  SPIFFS.remove("/save.csv");
  write_title = true;
  digitalWrite(BUILTIN_LED, LOW);
  delay(50);
  digitalWrite(BUILTIN_LED, HIGH);
  delay(50);

}
void save_on() {
  if (save.equals("true")) {
    save = "false";
  } else {
    save = "true";

  }

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
      delay(500);

      digitalWrite(D0, LOW);
      ESP.reset();
      delay(500);
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

  if (!client.connect(host_in, httpPort)) {
    Serial.print("Requesting URL error: ");
    //stream = "false";
    return;
  }

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

void read_sensors() {
  float  val_capt1 = sensor_start(capt1, pin_sensor1);
  float val_capt2 = sensor_start(capt2, pin_sensor2);
  Serial.println("lecture des capteurs");
  Serial.print("capteur 1 : ");
  Serial.println(val_capt1);
  Serial.print("capteur 2 : ");
  Serial.println(val_capt2);
  delay(100);
}
void web_update() {
  t_httpUpdate_return ret = ESPhttpUpdate.update("192.30.253.112", 80, "/julienrat/captobox/raw/master/bin/captobox.bin", "optional current version string here");
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.println("[update] Update failed.");
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[update] Update no Update.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[update] Update ok."); // may not called we reboot the ESP
      break;
  }
}

