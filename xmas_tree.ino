#include <WS2812FX.h>
//#define AT_HOME
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
#ifdef AT_HOME
#define LED_COUNT 16
#define FIRST_USABLE_LED 2
#define LAST_USABLE_LED 13
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
#else
#define LED_COUNT 300
#define FIRST_USABLE_LED 36
#define LAST_USABLE_LED 262
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

#define INPUT_BUFFER_MAX 64

char in_char;
char in_buff[INPUT_BUFFER_MAX];
uint8_t in_count = 0;

uint32_t fx_color = 0xff0000;
uint8_t fx_mode = 12;
uint8_t fx_brightness = 10;

void update_tree() {
  ws2812fx.setSegment( 0, 0,                    FIRST_USABLE_LED - 1, FX_MODE_STATIC, (uint32_t)0x000000, 1000, false );
  ws2812fx.setSegment( 1, FIRST_USABLE_LED,     LAST_USABLE_LED,      fx_mode,        (uint32_t)fx_color, 1000, false );
  ws2812fx.setSegment( 2, LAST_USABLE_LED + 1,  LED_COUNT-1,          44,             (uint32_t)0xFF4000, 1000, false );
  ws2812fx.setBrightness( fx_brightness );
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
  Serial.print(PROMPT);
}



void submit_command( char* s ) {
  switch ( s[0] )
  {
    case '\0':
      Serial.write('X');
      break;

    case 'b':
      fx_brightness = atol(&(s[1]));
      Serial.write('B');
      break;

    case 'p':
      fx_mode = atol(&s[1]);
      Serial.write('P');
      break;

    case 'c':
      char *endPtr;
      fx_color = strtol( &s[1], &endPtr, 16 );
      Serial.write('C');
      break;

    case 's':
      Serial.write( "Sp");
      Serial.print( fx_mode );
      Serial.write( ";c" );
      Serial.print( fx_color, HEX );
      Serial.write( ";b" );
      Serial.print( fx_brightness );
      Serial.write( '\n' );
      break;

    default:
      Serial.print("X");
  }
  
  update_tree();
  Serial.print("\n" PROMPT);
}

void loop() {
  ws2812fx.service();
  if ( Serial.available() ) {
    in_char = Serial.read();
    Serial.write( in_char );
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
  }
}
