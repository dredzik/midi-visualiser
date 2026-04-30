#include <Arduino.h>
#include <BayIndicator.h>

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

#include "Adafruit_GrayOLED.h"

#define MIDI_RX_PIN D7
#define MIDI_TX_PIN D6

BLEMIDI_CREATE_DEFAULT_INSTANCE();

uint8_t notes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
bool dirty = false;

BayIndicator display1 = BayIndicator(D4, D5, D3);
BayIndicator display2 = BayIndicator(D9, D8, D10);

static int8_t DRUM_MAP[128];

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  int8_t note = DRUM_MAP[pitch];
  uint8_t value = (velocity / 127.) * 192;

  dirty = true;

  if (note != -1) {
    notes[note] = value;
  }

  Serial.printf("Ch:%d Note:%d (%d) Vel:%d (%d)\r\n", channel, pitch, note, velocity, value);
}

void setup() {
  memset(DRUM_MAP, -1, sizeof(DRUM_MAP));

  // Set non-default values
  DRUM_MAP[38] = 0; // snare
  DRUM_MAP[40] = 0; // snare
  DRUM_MAP[36] = 0; // kick drum

  DRUM_MAP[48] = 1; // tom high
  DRUM_MAP[50] = 1; // tom high

  DRUM_MAP[45] = 2; // tom mid
  DRUM_MAP[47] = 2; // tom mid

  DRUM_MAP[43] = 3; // tom low
  DRUM_MAP[58] = 3; // tom low


  DRUM_MAP[22] = 4; // hi-hat closed
  DRUM_MAP[26] = 4; // hi-hat open
  DRUM_MAP[42] = 4; // hi-hat closed
  DRUM_MAP[44] = 4; // hi-hat pedal
  DRUM_MAP[46] = 4; // hi-hat open

  DRUM_MAP[51] = 5; // ride
  DRUM_MAP[53] = 5; // ride bell
  DRUM_MAP[59] = 5; // ride

  DRUM_MAP[55] = 6; // crash 1
  DRUM_MAP[49] = 6; // crash 1

  DRUM_MAP[52] = 7; // crash 2
  DRUM_MAP[57] = 7; // crash 2

  Serial.begin(115200);

  MIDI.begin();
  MIDI.setHandleNoteOn(handleNoteOn);
  Serial.println("MIDI ready.");

  display1.begin();
  display1.fillScreen(MONOOLED_WHITE);
  display1.display();

  display2.begin();
  display2.fillScreen(MONOOLED_WHITE);
  display2.display();
}

void loop() {
  MIDI.read();

  if (dirty) {
    dirty = false;

    display1.fillScreen(MONOOLED_BLACK);
    display2.fillScreen(MONOOLED_BLACK);

    for (int i = 0; i < 8; i++) {
      display1.fillRect(0, i, notes[i], 1, MONOOLED_WHITE);
      display2.fillRect(0, i, notes[i], 1, MONOOLED_WHITE);

      if (!notes[i]) {
        continue;
      }

      dirty = true;

      notes[i] = notes[i] > 4 ? notes[i] - 4 : 0;
    }

    display1.display();
    display2.display();
  }
}
