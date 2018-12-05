#include <WS2812FX.h>

#define PROMPT "XMOS 0.1 >"

#define LED_COUNT 300
#define LED_PIN 2

#define TIMER_MS 5000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long last_change = 0;
unsigned long now = 0;

void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(5);
  ws2812fx.setSegment( 0, 0, 30, FX_MODE_STATIC, (uint32_t)0x000000, 1000, false );
  ws2812fx.setSegment( 1, 31, 268, 22, (uint32_t)0xFF7000, 1000, false );
  ws2812fx.setSegment( 2, 269, 271, FX_MODE_STATIC, (uint32_t)0x000000, 1000, false );
  ws2812fx.setSegment( 2, 272, 300, 44, (uint32_t)0xFF4000, 1000, false );

  ws2812fx.start();

  Serial.begin(9600);
  while (!Serial) {}; // wait for connect
  Serial.print(PROMPT);
}

#define INPUT_BUFFER_MAX 256

char in_char;
char in_buff[INPUT_BUFFER_MAX];
uint8_t in_count = 0;

uint32_t color = 0xff7b00;
uint8_t fx_mode = FX_MODE_STATIC;


void submit_command( char* s ) {
  uint32_t operand = 0;
  switch ( s[0] )
  {
    case 'b':
      operand = atol(&s[1]);
      Serial.print( "brightness: " );
      Serial.print( operand );
      ws2812fx.setBrightness(operand);
      break;

    case 'p':
      operand = atol(&s[1]);
      Serial.print( "program: ");
      Serial.print( operand );
      fx_mode = operand;
      break;

    case 'c':
      char *endPtr;
      operand = strtol( &s[1], &endPtr, 16 );
      Serial.print( "color: ");
      Serial.print( operand );
      color = operand;
      break;
  }
  Serial.print("\n" PROMPT);
  ws2812fx.setSegment( 1, 31, 268, fx_mode, (uint32_t)color, 1000, false );
}

void loop() {
  now = millis();

  ws2812fx.service();

  while ( Serial.available() ) {
    in_char = Serial.read();
    switch (in_char) {
      case '\n':
        in_buff[in_count] = '\0';
        submit_command( in_buff );
        in_count = 0;
        break;
      case '\r':
        // discard
        break;
      default:
        if ( in_count < INPUT_BUFFER_MAX ) {
          in_buff[in_count++] = in_char;
        }
    }
  }
}
