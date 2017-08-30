#include <DallasTemperature.h>
#include <Adafruit_BMP085_U.h>
#include <DHT.h>

#include <iAQ-MOD1023.h>
#include <Wire.h>
#include "BlueDot_BME280.h"


typedef  const int pin;
class dht11_temp {
  public:
    dht11_temp();
    float read_temp(pin);
};
dht11_temp::dht11_temp() {};
float dht11_temp::read_temp(int pin) {
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
  float sound_buffer = 0;
  for (int i = 0; i < 100; i++) {
    sound_buffer = sound_buffer + analogRead(A0);
    //delay(1);
  }
  return sound_buffer / 100;
};

class mod_cov {
  public:
    mod_cov();
    float read_cov(pin);
};
mod_cov::mod_cov() {};
float mod_cov::read_cov(int pin) {
  if (pin == RX) {
    Wire.begin(TX, RX);
    Serial.println("mod-1023 sur entree tx");

  } else {
    Wire.begin();

  }
  Wire.setClockStretchLimit(1500);
  iaq.readRegisters();
  return iaq.getTVOC();
};

class mod_co2 {
  public:
    mod_co2();
    float read_mod_co2(pin);
};
mod_co2::mod_co2() {};
float mod_co2::read_mod_co2(int pin) {
  if (pin == RX) {
    Wire.begin(TX, RX);


  } else {
    Wire.begin();

  }
  Wire.setClockStretchLimit(1500);
  iaq.readRegisters();
  return iaq.getPrediction();
};

class mod_temp {
  public:
    mod_temp();
    float read_mod_temp(pin);
};
mod_temp::mod_temp() {};
float mod_temp::read_mod_temp(int pin) {
  BlueDot_BME280 bme280 = BlueDot_BME280();
  bme280.parameter.communication = 0;
  bme280.parameter.I2CAddress = 0x76;
  bme280.parameter.sensorMode = 0b11;
  bme280.parameter.IIRfilter = 0b100;
  bme280.parameter.humidOversampling = 0b101;
  bme280.parameter.tempOversampling = 0b101;
  bme280.parameter.pressOversampling = 0b101;
  bme280.init();
  
  return bme280.readTempC();

};

class mod_hum {
  public:
    mod_hum();
    float read_mod_hum(pin);
};
mod_hum::mod_hum() {};
float mod_hum::read_mod_hum(int pin) {
  BlueDot_BME280 bme280 = BlueDot_BME280();
  bme280.parameter.communication = 0;
  bme280.parameter.I2CAddress = 0x76;
  bme280.parameter.sensorMode = 0b11;
  bme280.parameter.IIRfilter = 0b100;
  bme280.parameter.humidOversampling = 0b101;
  bme280.parameter.tempOversampling = 0b101;
  bme280.parameter.pressOversampling = 0b101;
  bme280.init();
  
  return bme280.readHumidity();
};

class mod_bme280 {
  public:
    mod_bme280();
    float read_mod_bme280(pin);
};
mod_bme280::mod_bme280() {};
float mod_bme280::read_mod_bme280(int pin) {
   BlueDot_BME280 bme280 = BlueDot_BME280();
  bme280.parameter.communication = 0;
  bme280.parameter.I2CAddress = 0x76;
  bme280.parameter.sensorMode = 0b11;
  bme280.parameter.IIRfilter = 0b100;
  bme280.parameter.humidOversampling = 0b101;
  bme280.parameter.tempOversampling = 0b101;
  bme280.parameter.pressOversampling = 0b101;
  bme280.init();
  
  return bme280.readPressure();

};

class dust {
  public:
    dust();
    float read_dust(pin);
};
dust::dust() {};
float dust::read_dust(int pin) {
#define I2C_SLAVE_ADDR_TEMP   0x27         // I2C Temperature slave address
#define DELAY_TEMP_READ   140              // wait 140 ms before ask temp value
#define DELAY_NEW_SAMPLE 1000              // time before new sample (1s)
  float dustValue;
  byte* floatPtr;
  float tempValue;
  if (pin == RX) {
    Wire.begin(TX, RX);
  } else {
    Wire.begin();
  }

  Wire.setClockStretchLimit(1500);
  Wire.beginTransmission(I2C_SLAVE_ADDR_TEMP);  // transmit to slave device
  Wire.write(0xFF);                             // sends usuless data
  Wire.endTransmission();                       // stop transmitting

  delay(DELAY_TEMP_READ);

  Wire.requestFrom(I2C_SLAVE_ADDR_TEMP, 4);     // request 4 byte from slave (float = 4 bytes)

  if (Wire.available()) {
    floatPtr = (byte*) &tempValue;
    (*floatPtr) = Wire.read(); ++floatPtr;
    (*floatPtr) = Wire.read(); ++floatPtr;
    (*floatPtr) = Wire.read(); ++floatPtr;
    (*floatPtr) = Wire.read();

    return tempValue;
  } else {

    return 100000;
  }

  delay(DELAY_NEW_SAMPLE);
};






float sensor_start(String type, int pin) {
  if (type.equals("dht11_temp")) {
    dht11_temp temp;
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
  } else   if (type.equals("mod_cov")) {
    mod_cov val_cov;
    return val_cov.read_cov(5);
  } else   if (type.equals("mod_co2")) {
    mod_co2 val_co2;
    return val_co2.read_mod_co2(5);
  } else   if (type.equals("mod_temp")) {
    mod_temp val_temp;
    return val_temp.read_mod_temp(5);
  } else   if (type.equals("mod_hum")) {
    mod_hum val_hum;
    return val_hum.read_mod_hum(5);
  } else   if (type.equals("mod_bme280")) {
    mod_bme280 val_bme280;
    return val_bme280.read_mod_bme280(5);
  } else   if (type.equals("dust")) {
    dust val_dust;
    return val_dust.read_dust(pin);
  } else {
    return 0;
  }
}


String sensor_name(String type) {
  if (type.equals("dht11_temp")) {

    return "Température";
  } else   if (type.equals("co2")) {
    return "Dioxyde de carbone";
  } else   if (type.equals("light")) {
    return "Lumière";
  } else   if (type.equals("sound")) {
    return "Son";
  } else   if (type.equals("mod_cov")) {
    return "COV";
  } else   if (type.equals("mod_co2")) {
    return "Dioxyde de carbone";
  } else   if (type.equals("mod_temp")) {
    return "Température";
  } else   if (type.equals("mod_hum")) {
    return "Humidité";
  } else   if (type.equals("mod_bme280")) {
    return "Pression";
  } else   if (type.equals("dust")) {
    return "Particules";
  } else {
    return "Pas de capteur";
  }

}

String sensor_min(String type) {
  if (type.equals("dht11_temp")) {
    return "-10";
  } else   if (type.equals("co2")) {
    return "0";
  } else   if (type.equals("light")) {
    return "0";
  } else   if (type.equals("son")) {
    return "0";
  } else   if (type.equals("mod_cov")) {
    return "125";
  } else   if (type.equals("mod_co2")) {
    return "450";
  } else   if (type.equals("mod_temp")) {
    return "-40";
  } else   if (type.equals("mod_hum")) {
    return "0";
  } else   if (type.equals("mod_bme280")) {
    return "300";
  } else   if (type.equals("dust")) {
    return "0";
  } else {
    return "0";
  }

}

String sensor_max(String type) {
  if (type.equals("dht11_temp")) {
    return "50";
  } else   if (type.equals("co2")) {
    return "2000";
  } else   if (type.equals("light")) {
    return "180";
  } else   if (type.equals("sound")) {
    return "180";
  } else   if (type.equals("mod_cov")) {
    return "600";
  } else   if (type.equals("mod_co2")) {
    return "2000";
  } else   if (type.equals("mod_temp")) {
    return "85";
  } else   if (type.equals("mod_hum")) {
    return "100";
  } else   if (type.equals("mod_bme280")) {
    return "1100";
  } else   if (type.equals("dust")) {
    return "200";
  }  else {
    return "100";
  }

}


