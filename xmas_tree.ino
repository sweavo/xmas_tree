#include <WS2812FX.h>

#define PROMPT "XMOS 0.1>"

#define LED_COUNT 300
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
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


#define INPUT_BUFFER_MAX 64

char in_char;
char in_buff[INPUT_BUFFER_MAX];
uint8_t in_count = 0;

uint32_t color = 0xff7b00;
uint8_t fx_mode = 12;

void setup() {

  ws2812fx.init();
  ws2812fx.setBrightness(5);
  ws2812fx.setSegment( 0, 0, 28, FX_MODE_STATIC, (uint32_t)0x000000, 1000, false );
  ws2812fx.setSegment( 1, 29, 262, fx_mode, (uint32_t)0xFF7000, 1000, false );
  ws2812fx.setSegment( 2, 263, 299, 44, (uint32_t)0xFF4000, 1000, false );
//  ws2812fx.setSegment( 2, 263, 299, 1, (uint32_t)0xFFFF80, 1000, false );
  ws2812fx.setBrightness(10);

  ws2812fx.start();

  Serial.begin(9600);
  while (!Serial) {}; // wait for connect
  while ( Serial.available() ) {Serial.read(); }; // flush
  Serial.print(PROMPT);
}



void submit_command( char* s ) {
  uint32_t operand = 0;
  switch ( s[0] )
  {
    case '\0':
      break;
    case 'b':
      operand = atol(&(s[1]));
      ws2812fx.setBrightness(operand);
      break;

    case 'p':
      operand = atol(&s[1]);
      fx_mode = operand;
      break;

    case 'c':
      char *endPtr;
      operand = strtol( &s[1], &endPtr, 16 );
      color = operand;
      break;

    default:
      Serial.print("no comprendo");
  }
  ws2812fx.setSegment( 1, 29, 262, fx_mode, (uint32_t)color, 1000, false );
  Serial.print("\n" PROMPT);
}

void loop() {
  ws2812fx.service();
  if ( Serial.available() ) {
    in_char = Serial.read();
    Serial.write( in_char );
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
        if ( in_count < INPUT_BUFFER_MAX-1 ) {
          in_buff[in_count++] = in_char;
        }
    }
  }
}
