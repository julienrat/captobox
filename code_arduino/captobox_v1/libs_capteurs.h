#include <DallasTemperature.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>
typedef  const int pin;
class dht11 {
  public:
    dht11();
    float read_temp(pin);
};
dht11::dht11() {};
float dht11::read_temp(int pin) {
  DHT dht(pin, DHT11);
  dht.begin();
  return dht.readTemperature();
};

class co2 {
  public:
    co2();
    float read_co2();
};
co2::co2() {};
float co2::read_co2() {
  const unsigned char cmd_get_sensor[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  int size = sizeof(cmd_get_sensor);
  for (int i = 0; i < size; i++) Serial.write(cmd_get_sensor[i]);
  long cnt_timeout = 0;
  if (!Serial.available())
  { // wait for data
    delay(2);
    cnt_timeout++;
    if (cnt_timeout > 1000);       // time out
  }
  int len = 0;
  unsigned char data[20];
  while (Serial.available())
  {
    data[len++] = Serial.read();
  }
  if ((9 == len) && (cmd_get_sensor[0] == data[0]) && (cmd_get_sensor[2] == data[1]))
  { // data ok
    return (256 * data[2] + data[3]);
  }
};
class light {
  public:
    light();
    float read_light();
};
light::light() {};
float light::read_light() {
  pinMode(A0, INPUT);
  return analogRead(A0);
};

class sound {
  public:
    sound();
    float read_sound();
};
sound::sound() {};
float sound::read_sound() {
  pinMode(A0, INPUT);
  float sound_buffer=0;
  for(int i=0;i<100;i++){
    sound_buffer=sound_buffer+analogRead(A0);
    //delay(1);
  }
  return sound_buffer/100;
};


float sensor_start(String type, int pin) {
  if (type.equals("dht11")) {
    dht11 temp;
    return temp.read_temp(pin);
  } else   if (type.equals("co2")) {
    co2 val_co2;
    return val_co2.read_co2();
  } else   if (type.equals("light")) {
    light val_light;
    return val_light.read_light();
  } else   if (type.equals("sound")) {
    sound val_sound;
    return val_sound.read_sound();
  } else {
    return 0;
  }
}


String sensor_name(String type) {
  if (type.equals("dht11")) {

    return "température";
  } else   if (type.equals("co2")) {
    return "dioxyde de carbone";
  } else   if (type.equals("light")) {
    return "lumière";
  } else   if (type.equals("sound")) {
    return "son";
  } else {
    return "pas de capteur";
  }

}

String sensor_min(String type) {
  if (type.equals("dht11")) {
    return "-10";
  } else   if (type.equals("co2")) {
    return "0";
  } else   if (type.equals("light")) {
    return "0";
  } else   if (type.equals("son")) {
    return "0";
  } else {
    return "0";
  }

}

String sensor_max(String type) {
  if (type.equals("dht11")) {
    return "50";
  } else   if (type.equals("co2")) {
    return "2000";
  } else   if (type.equals("light")) {
    return "180";
  } else   if (type.equals("sound")) {
    return "180";
  } else {
    return "100";
  }

}


