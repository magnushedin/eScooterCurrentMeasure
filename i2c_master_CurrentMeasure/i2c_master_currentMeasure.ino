// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>
#include <TimerOne.h>
//#include <EEPROM.h>

//#define EE_ADDRESS_BATT_LEVEL 0

typedef union I2C_float_packet {
  float float_value;
  byte byte_array[4];
} I2C_float_packet_t;

const int input_pin = A0;
//const int button_pin = 2;
I2C_float_packet_t current;
I2C_float_packet_t battery_level;
int led_status = LOW;
int current_array_idx = 0;
byte send_byte;
int current_sensor_value_array[10];
float stored_battery_level = 0;
int latest_sensor_value = 0;

// Conversion of from one range to another
float map_float(float in, float in_min, float in_max, float out_min, float out_max) {
  return (float)((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

void setup() {
  current.float_value = 0;
    
  pinMode(input_pin, INPUT);
  //pinMode(button_pin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
    
  Timer1.initialize(10000);
  Timer1.attachInterrupt(callback);
  
  Wire.begin(); // join i2c bus (address optional for master)

  Serial.begin(115200);
  Serial.println("Init");

  //EEPROM.write(EE_ADDRESS_BATT_LEVEL, 6000);
  //stored_battery_level = EEPROM.read(EE_ADDRESS_BATT_LEVEL);
  battery_level.float_value = 0;
}

void callback()
{
  int sensor_value = 0;
  int sensor_raw_value = 0;
  // Read sensor
  sensor_raw_value = analogRead(input_pin);
  current_sensor_value_array[current_array_idx] = sensor_raw_value;
  latest_sensor_value = sensor_raw_value;
  
  current_array_idx++;
  if (current_array_idx == 10) {
    current_array_idx = 0;
  }
  
  for (int i = 0; i<10; i++) {
    sensor_value += current_sensor_value_array[i];
  }
  sensor_value = sensor_value/10.0;

  current.float_value  = map_float(sensor_value,0,1023,-20.0,20.0);

  battery_level.float_value = battery_level.float_value + current.float_value/360.0;

}

void loop() {
  //Serial.println("Begin transmission");
  Wire.beginTransmission(8); // transmit to device #8
  Wire.write(current.byte_array[0]);
  Wire.write(current.byte_array[1]);
  Wire.write(current.byte_array[2]);
  Wire.write(current.byte_array[3]);
  Wire.write(battery_level.byte_array[0]);
  Wire.write(battery_level.byte_array[1]);
  Wire.write(battery_level.byte_array[2]);
  Wire.write(battery_level.byte_array[3]);
  Wire.endTransmission();    // stop transmitting

  /*
  Serial.print("Current: ");
  Serial.print("( ");
  Serial.print(latest_sensor_value);
  Serial.print(" )");
  Serial.println(current.float_value);
  Serial.print("Battery_level: ");
  Serial.println(battery_level.float_value);
  Serial.println("End transmission");
  */

  // Blink the led
  led_status = !led_status;
  digitalWrite(LED_BUILTIN, led_status);
  
  delay(100);
}
