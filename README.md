# Solar Powered LoRaWAN Sensor Node

## Overview

This project is a solar-powered LoRaWAN sensor node designed to monitor environmental conditions such as soil moisture, temperature, humidity, and CO2 levels. The node transmits data via LoRaWAN to The Things Network (TTN), and the data can be visualized using platforms like DataCake and Home Assistant.

## Repository Structure

- **src**: Contains the source code for the sensor node.
- **hardware**: Contains the schematic of the project along with CAD STL files for the Solar Panel and Component Enclosure and the Stevenson Enclosure for the SCD30 Sensor.

## Libraries Used

The following libraries, are used in this project:
- `SCD30` by [Seeed Studio](https://github.com/Seeed-Studio/Seeed_SCD30) 
- `HardwareSerial` - Built in Libaray

## Setup

### 1. Hardware Setup
- Assemble the hardware components as per the schematic provided in the hardware folder.
- Use the CAD STL files to 3D print the enclosures for the solar panel, component, and Stevenson enclosure for the SCD30 sensor. (Recommended 3D Printing Filament : PETG)


### 2. Software Setup
- Open the code from the `src` folder in your Arduino IDE.
- Configure the necessary parameters, such as the App Key, in the code.
- Upload the code to the SeeedStudio XIAO ESP32-C3 microcontroller.

### 3. Integrate with The Things Network (TTN)
- Register your device on the TTN console and obtain the necessary credentials (Device EUI, Application EUI, and App Key).
- Configure the payload format to match the data structure sent by the sensor node.
- Add a webhook for DataCake integration to visualize the data.
- You have to also add the custom javascript format for decoding the HEX code sent from the LoRa Device on to your TTN and DataCake Dashboard: 

```js
function Decoder(bytes, port) {
  var decoded = {};
  if (port === 8) {
    decoded.soilMoisture = (bytes[0] << 8) | bytes[1];
    decoded.temp = ((bytes[2] << 8) | bytes[3]); // Assuming temperature was multiplied by 10 before sending
    decoded.humi = (bytes[4] << 8) | bytes[5];
    decoded.co2 = (bytes[6] << 8) | bytes[7];
    decoded.battery = (bytes[8] << 8) | bytes[9];
  }
  return decoded;
}
```

### 4. Integrate with Home Assistant
- Generate an API Key in the TTN console.
- Add "The Things Network" integration in Home Assistant and enter the necessary credentials.
- Configure the sensor entities in the `configuration.yaml` file to define the units of measurement:

  ```yaml
  sensor.lora_sense_node_temperature:
    unit_of_measurement: "°C"
  sensor.lora_sense_node_co2:
    unit_of_measurement: "ppm"
  sensor.lora_sense_node_humidity:
    unit_of_measurement: "%"
  sensor.lora_sense_node_soilmoisture:
    unit_of_measurement: "%"
  ```

## Future Improvements

Future improvements to this project could include integrating an external RTC with a power latch circuit to reduce idle power consumption. Additionally, more sensors can be added to extend the functionality of the sensor node for various applications.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.