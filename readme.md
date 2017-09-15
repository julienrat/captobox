# La Captobox

## Prérequis
- Disposer de la derniere version de l'IDE Arduino (1.8.4)
- Avoir installé les cartes ESP8266 en ajoutant l'adresse suivante dans le menu `Préférences` : `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
- Avoir installé le plugin ESP8266 file uploader https://github.com/esp8266/arduino-esp8266fs-plugin esp8266fs.jar dans le repertoire `<home_dir>/Arduino/tools/ESP8266FS/tool/`

## Installation
1. Téléchargez l'archive et dézzipez là dans le dossier arduino (là où se trouvent vos programmes, sous linux ~/Arduino)
2. Renommer le dossier `captobox-master` en `captobox`
3. Relancez l'IDE Arduino
4. Sélectionnez la carte `WeMos D1 R2 & mini`
5. Chargez le code de la captobox `Fichier / Carnet de croquis / captobox / captobox_beta`

## Liste des librairies à installer
Pour installer les librairies rendez-vous dans `Croquis / Inclure un bibliothèque / Ajouter une bibliothèque ZIP...`
, dans le dossier `~/Arduino/captobox/libs` ajoutez une à une les bibliothèques zippées
- Adafruit_BMP085_Unified.zip
- Adafruit_Unified_Sensor.zip
- BlueDot_BME280_Library.zip
- BME280-master.zip
- DallasTemperature.zip
- DHT_sensor_library.zip
- elapsedMillis.zip
- MOD-1023-master.zip
- OneButton.zip
- OneWire.zip
- Time.zip

## Compilez ! et téléversez !
