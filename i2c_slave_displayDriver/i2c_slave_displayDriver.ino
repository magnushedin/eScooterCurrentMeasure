/**************************************************************************
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <Wire.h>

#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

typedef union I2C_float_packet_t {
  float float_value;
  byte byte_array[4];
} I2C_float_packet;

I2C_float_packet current;
I2C_float_packet battery_level;
float voltage = 40;
int current_log[10] = {0};
int current_screen_buffer[240] = {0};
int current_rect = 0;
int battery_level_rect = 0;
int current_log_idx = 0;
int current_screen_buffer_idx = 0;
int w = 0;


void receiveEvent(int howMany) {
  //Serial.println("Message received");
  current.byte_array[0] = Wire.read();
  current.byte_array[1] = Wire.read();
  current.byte_array[2] = Wire.read();
  current.byte_array[3] = Wire.read();

  battery_level.byte_array[0] = Wire.read();
  battery_level.byte_array[1] = Wire.read();
  battery_level.byte_array[2] = Wire.read();
  battery_level.byte_array[3] = Wire.read();

  current_log[current_log_idx] = (int)current.float_value;
  current_log_idx++;
  if (current_log_idx == 10) {
    current_log_idx = 0;
  }
}

void setup(void) {
  Serial.begin(115200);
  Serial.println(F("Booting"));

  Wire.begin(8);
  Wire.onReceive(receiveEvent);

  // Init screen
  tft.init(240, 240);           // Init ST7789 240x240
  tft.setTextWrap(false);
  tft.setRotation(1);

  // Set screen black
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  // Zero variables
  current.float_value = 0;
  battery_level.float_value = 0;

  Serial.println(F("Initialized"));
}

// Conversion of from one range to another
int my_map(float in, float in_min, float in_max, float out_min, float out_max) {
  return (int)((((in - in_min) / (in_max - in_min)) * (out_max - out_min)) + out_min);
}

// Update current rectange on screen
void update_current_rect(float current) {
  static float previous_current = 0.0;
  int from, to;
  if (previous_current < current) {
    from = my_map(previous_current, 0, 20 , 0, tft.width());
    to = my_map(current, 0, 20, 0, tft.width());
    tft.fillRect(from, 30, to - from, 22, ST77XX_YELLOW);
  }
  else if (previous_current > current) {
    from = my_map(current, 0, 20 , 0, tft.width());
    to = my_map(previous_current, 0, 20, 0, tft.width());
    tft.fillRect(from, 30, to - from, 22, ST77XX_BLACK);
  }
  previous_current = current;
}

// Update battery level rectangle on screen
void update_battery_level_rect(float battery_level) {
  static float previous_level = 0;
  int from, to;
  if (previous_level < battery_level) {
    from = my_map(previous_level, 0, 6600 , 0, tft.width());
    to = my_map(battery_level, 0, 6600, 0, tft.width());
    tft.fillRect(from, 90, to - from, 22, ST77XX_GREEN);
  }
  else if (previous_level > battery_level) {
    from = my_map(battery_level, 0, 6600 , 0, tft.width());
    to = my_map(previous_level, 0, 6600, 0, tft.width());
    tft.fillRect(from, 90, to - from, 22, ST77XX_BLACK);
  }
  previous_level = battery_level;
}


// Update log screen buffer
void update_log_screen_buffer()
{
  int new_value = 0;
  for (int i = 0; i < 10; i++) {
    new_value += current_log[i];
  }
  new_value = new_value/10;
  
  /* debug
  new_value = random(current_screen_buffer[0]-5,current_screen_buffer[0]+5);
  if (new_value < 0) {
    new_value = 0;
  }
  if (new_value > 15) {
    new_value = 15;
  }
  */

  for (int i=239;i>=0;i--) {
    current_screen_buffer[i] = current_screen_buffer[i-1];
  }
  current_screen_buffer[0] = new_value;
}


// Plot buffer to screen
void plot_log_screen_buffer(int color)
{
  int y_axis = 0;
  int y_axis_old = 0;
  for (int x_axis=0;x_axis<240; x_axis++) {
    //y_axis = 239-(7*15); //debug
    y_axis = 239-(7*current_screen_buffer[x_axis]);
    //y_axis_old = 239-(7*current_screen_buffer[x_axis+1]);
    tft.drawPixel(x_axis, y_axis, color);
    //tft.drawLine(x_axis, y_axis, x_axis+1, y_axis_old, color);
  }
}

void loop() {  
  // Current
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_YELLOW);
  tft.fillRect(0, 0, 239, 22, ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(0, 0);
  tft.print(current.float_value);
  tft.print(" (");
  tft.print((int)(current.float_value * voltage));
  tft.println("W)");
  update_current_rect(current.float_value);

  // Battery level
  tft.setTextSize(3);
  tft.fillRect(0, 60, 150, 22, ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(0, 60);
  //tft.println((int)current_screen_buffer[0]);
  tft.println((int)battery_level.float_value);
  update_battery_level_rect(battery_level.float_value);

  // Plot the log
  if (w >= 2) {
    w=0;
    plot_log_screen_buffer(ST77XX_BLACK);
    update_log_screen_buffer();
    plot_log_screen_buffer(ST77XX_WHITE);
  }
  w++;

   
  // Debug
  //Serial.println("Screen up to date");
  delay(100);
}
