#include <Arduino.h>
#include <BayIndicator.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

#include "Adafruit_GrayOLED.h"

#define MIDI_RX_PIN D7
#define MIDI_TX_PIN D6

BLEMIDI_CREATE_DEFAULT_INSTANCE();

BayIndicator display1 = BayIndicator(D4, D5, D3);
BayIndicator display2 = BayIndicator(D9, D8, D10);

static int8_t DRUM_MAP[128];

uint8_t filter = 127;
uint8_t notes[64] = {0};
int8_t current_note = -1;
bool dirty = false;

void handleProgramChange(byte channel, byte value) {
  filter = value;

  Serial.printf("Channel: %d Value: %d\r\n", channel, value);
}

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (filter != 127 && filter != DRUM_MAP[pitch]) {
    return;
  }

  dirty = true;
  current_note++;

  if (current_note >= 64) {
    current_note = 0;
  }

  if (pitch != -1) {
    notes[current_note] = velocity;
  }

  Serial.printf("Channel: %d Pitch: %d Velocity: %d\r\n", channel, pitch, velocity);
}

void setup_drum_map() {
  memset(DRUM_MAP, -1, sizeof(DRUM_MAP));

  DRUM_MAP[38] = 0; // snare
  DRUM_MAP[40] = 0; // snare
  DRUM_MAP[36] = 0; // kick drum

  DRUM_MAP[48] = 1; // tom high
  DRUM_MAP[50] = 1; // tom high

  DRUM_MAP[45] = 1; // tom mid
  DRUM_MAP[47] = 1; // tom mid

  DRUM_MAP[43] = 1; // tom low
  DRUM_MAP[58] = 1; // tom low

  DRUM_MAP[22] = 2; // hi-hat closed
  DRUM_MAP[26] = 2; // hi-hat open
  DRUM_MAP[42] = 2; // hi-hat closed
  DRUM_MAP[44] = 2; // hi-hat pedal
  DRUM_MAP[46] = 2; // hi-hat open

  DRUM_MAP[51] = 2; // ride
  DRUM_MAP[53] = 2; // ride bell
  DRUM_MAP[59] = 2; // ride

  DRUM_MAP[55] = 1; // crash 1
  DRUM_MAP[49] = 1; // crash 1

  DRUM_MAP[52] = 1; // crash 2
  DRUM_MAP[57] = 1; // crash 2
}


void setup() {
  setup_drum_map();

  Serial.begin(115200);

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_BT);

  char name[32];
  snprintf(name, sizeof(name), "midi-visualiser-%02x%02x%02x%02x", mac[5], mac[4], mac[3], mac[2]);
  BLEMIDI.setName(name);

  MIDI.begin();
  MIDI.setHandleProgramChange(handleProgramChange);
  MIDI.setHandleNoteOn(handleNoteOn);
  Serial.println("MIDI ready.");

  display1.begin();
  display2.begin();

  display1.fillScreen(MONOOLED_WHITE);
  display2.fillScreen(MONOOLED_WHITE);

  display1.display();
  display2.display();
}

void animation() {
  if (!dirty) {
    return;
  }

  uint8_t travel = 4;
  dirty = false;

  for (uint8_t i = 0; i < 64; i++) {
    uint8_t width = 192 * (notes[i] / 127.);
    uint8_t height = 9;
    uint8_t start_y = 0;
    uint8_t start_x = 0;
    BayIndicator &display = i % 2 ? display1 : display2;

    display.fillRect(start_x, start_y, width, height, MONOOLED_WHITE);

    if (notes[i] >= travel) {
      dirty = true;
      notes[i] -= travel;
    } else {
      notes[i] = 0;
    }
  }
}

void loop() {
  MIDI.read();
  vTaskDelay(1);

  display1.fillScreen(MONOOLED_BLACK);
  display2.fillScreen(MONOOLED_BLACK);
  animation();
  display1.display();
  display2.display();
}
