
// Function to send an AT command and check for a specific response

static int at_send_check_response(char *p_ack, int timeout_ms, char *p_cmd, ...) {
  int ch;
  int num = 0;
  int index = 0;
  int startMillis = 0;
  va_list args;
  memset(recv_buf, 0, sizeof(recv_buf));  // Clear the receive buffer
  va_start(args, p_cmd);
  Serial2.printf(p_cmd, args);  // Send command to LoRa module
  Serial.printf(p_cmd, args);   // Print command to serial monitor
  va_end(args);
  delay(200);
  startMillis = millis();

  if (p_ack == NULL) {
    return 0;  // Return if no acknowledgment string is provided
  }

  do {
    while (Serial2.available() > 0) {
      ch = Serial2.read();     // Read character from LoRa module
      recv_buf[index++] = ch;  // Store character in receive buffer
      Serial.print((char)ch);  // Print character to serial monitor
      delay(2);
    }

    if (strstr(recv_buf, p_ack) != NULL) {
      return 1;  // Return success if acknowledgment is found
    }

  } while (millis() - startMillis < timeout_ms);
  return 0;  // Return failure if timeout is reached
}

// Function to parse received messages from the LoRa module
static void recv_prase(char *p_msg) {
  if (p_msg == NULL) {
    return;
  }
  char *p_start = NULL;
  int data = 0;
  int rssi = 0;
  int snr = 0;

  p_start = strstr(p_msg, "RX");
  if (p_start && (1 == sscanf(p_start, "RX: \"%d\"\r\n", &data))) {
    Serial.println(data);
    led = !!data;
    if (led) {
      digitalWrite(LED_BUILTIN, LOW);  // Turn on LED if data received - Not used on hardware for power effiency
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // Turn off LED otherwise - Not used on hardware for power effiency
    }
  }

  p_start = strstr(p_msg, "RSSI");
  if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi))) {
    Serial.print(rssi);  // Print RSSI value
  }
  p_start = strstr(p_msg, "SNR");
  if (p_start && (1 == sscanf(p_start, "SNR %d", &snr))) {
    Serial.print("snr :");
    Serial.println(snr);  // Print SNR value
  }
}



void getSensors() {

  for (int i = 0; i < 4; i++) {

    soilSensorValue = analogRead(soilSensor);                     // Read soil moisture sensor
    soilMoisturePercent = map(soilSensorValue, 0, 4095, 0, 100);  // Convert to percentage

    float result[3] = { 0 };

    if (scd30.isAvailable()) {
      scd30.getCarbonDioxideConcentration(result);  // Read SCD30 sensor values
      Serial.print("Carbon Dioxide Concentration is: ");
      Serial.print(result[0]);
      Serial.println(" ppm");

      Serial.print("Temperature = ");
      Serial.print(result[1]);
      Serial.println(" ℃");

      Serial.print("Humidity = ");
      Serial.print(result[2]);
      Serial.println(" %");
      Serial.println(" ");

      temp = result[1];
      humi = result[2];
      co2 = result[0];
    }
    vTaskDelay(1000);  // Delay to allow sensor stabilization
  }

  scd30.stopMeasurement();  // Stops SCD30 from making measurement for power saving.


  float total_voltage = 0;
  // Take 20 readings for the battery voltage and average them
  for (int i = 0; i < 20; i++) {
    batterySensorValue = analogReadMilliVolts(batterySensor);
    float actual_voltage = ((batterySensorValue * ((R1 + R2) / R2))) / 1000;
    total_voltage += actual_voltage;
  }

  float average_voltage = total_voltage / 20;
  battery = (average_voltage / 4.3) * 100; // Calculate battery percentage
  Serial.print("Battery = ");
  Serial.println(battery);
}


// Function to send data over LoRa
void sendData() {

  if (is_exist) {

    int ret = 0;

    if (is_join) {

      // Send an AT command to join the LoRa network and check the response
      ret = at_send_check_response("+JOIN: Network joined", 12000, "AT+JOIN\r\n");

      // Prepare and send the sensor data as a hexadecimal string via LoRa
      char cmd[128];
      sprintf(cmd, "AT+CMSGHEX=\"%04X%04X%04X%04X%04X\"\r\n", (int)soilMoisturePercent, (int)temp, (int)humi, (int)co2, (int)battery);
      ret = at_send_check_response("Done", 5000, cmd);

      // Check if data was sent successfully and process the response
      if (ret) {
        recv_prase(recv_buf);  // Parse received message if data sent successfully
      } else {
        Serial.print("Send failed!\r\n\r\n"); // Print error message if sending failed
      }
    }
  }
  // Put LoRa module to sleep mode to save power
  at_send_check_response("+AT: OK", 100, "AT+LOWPOWER\r\n");  // Puts LoRa Module to sleep mode - reduces current from 7.82mA to 51.7uA
}