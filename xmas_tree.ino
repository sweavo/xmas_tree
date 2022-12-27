#include <WS2812FX.h>
#define PROMPT "XMOS 0.1>"

#define LED_PIN 4

#define TIMER_MS 5000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
#define LED_COUNT 69
#define STAR_LEDS 20
#define FIRST_USABLE_LED 2
#define LAST_USABLE_LED ((LED_COUNT) - (STAR_LEDS))
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#define INPUT_BUFFER_MAX 64

char in_char;
char in_buff[INPUT_BUFFER_MAX];
uint8_t in_count = 0;

uint16_t revert_in=0; // for alert function
uint8_t old_mode= 0; // for alert function
uint8_t fx_mode = 12; // unicorn thing
//uint8_t fx_mode = 47; // xmas
uint32_t fx_color = 0xff0000;
uint8_t fx_brightness = 10;

bool mode_dirty = true;
bool color_dirty = true;
bool brightness_dirty = true;

void update_tree() {
  ws2812fx.setSegment( 0, 0,                    FIRST_USABLE_LED - 1, FX_MODE_STATIC, (uint32_t)0x000000, 1000, false );
  ws2812fx.setSegment( 1, FIRST_USABLE_LED,     LAST_USABLE_LED,      fx_mode,        (uint32_t)fx_color, 1000, false );
  ws2812fx.setSegment( 2, LAST_USABLE_LED + 1,  LED_COUNT-1,          44,             (uint32_t)0xFF4000, 1000, false );
  ws2812fx.setBrightness( fx_brightness );
  ws2812fx.setSpeed(20);
}

void setup() {
  ws2812fx.init();
  update_tree();
  ws2812fx.start();

  Serial.begin(9600);
  while (!Serial) {}; // wait for connect
  while ( Serial.available() ) {
    Serial.read(); // flush
  };
  Serial.print("Setup complete\n");
  Serial.print(PROMPT);
}

void submit_command( char* s ) {
  switch ( s[0] )
  {
    case '\0':
      Serial.write('X');
      break;

    case 'm':
      fx_mode = atol(&s[1]);
      mode_dirty=true;
      break;

    case 'c':
      char *endPtr;
      fx_color = strtol( &s[1], &endPtr, 16 );
      color_dirty=true;
      break;

    case 'b':
      fx_brightness = atol(&(s[1]));
      brightness_dirty=true;
      break;

    case 's':
      color_dirty=true;
      brightness_dirty=true;
      mode_dirty=true;
      break;

    case 'a': //alert! --- go to a bright state for a couple of seconds
      old_mode=fx_mode;
      fx_mode=1;
      mode_dirty=true;
      revert_in=200; // 2 seconds, maybe?
      break;
        
    default:
      Serial.print("X");
  }
  
  Serial.print("\n" PROMPT);
}

void loop() {
  ws2812fx.service();
  delay(10);
  if ( Serial.available() ) {
    in_char = Serial.read();
    switch (in_char) {
      case '\n':
      case ';':
        in_buff[in_count] = '\0';
        submit_command( in_buff );
        in_count = 0;
        break;
      case '\r':
        // discard
        break;
      default:
        if ( in_count < INPUT_BUFFER_MAX - 1 ) {
          in_buff[in_count++] = in_char;
        }
    }
  } else {
    if ( revert_in ) {
      --revert_in;
      if ( ! revert_in ) {
        fx_mode=old_mode;
        mode_dirty=1;
      }
    }
    if (mode_dirty || color_dirty || brightness_dirty ){
      update_tree();
      if (mode_dirty){
        mode_dirty=false;
        Serial.write('m');
        Serial.print( fx_mode );
        Serial.write(';');
      }
      if (color_dirty){
        color_dirty=false;      
        Serial.write('c');
        Serial.print( fx_color, HEX );
        Serial.write(';');
      }
      if (brightness_dirty){
        brightness_dirty=false;
        Serial.write('b');
        Serial.print( fx_brightness );
        Serial.write(';');
      }
    }
  }
}
