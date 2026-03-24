#include <Arduino.h>
#include <BayIndicator.h>
#include <MIDI.h>

#include "Adafruit_GrayOLED.h"

#define MIDI_RX_PIN D7
#define MIDI_TX_PIN D6

HardwareSerial MidiSerial(1);
MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, MIDI);

uint8_t notes[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
bool dirty = false;

BayIndicator display1 = BayIndicator(D4, D5, D3);
BayIndicator display2 = BayIndicator(D9, D8, D10);

static const int8_t DRUM_MAP[128] = {
    //  0    1    2    3    4    5    6    7    8    9
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 0–9
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 10–19
       -1,  -1,   5,  -1,  -1,  -1,   5,  -1,  -1,  -1,  // 20–29
       -1,  -1,  -1,  -1,  -1,  -1,   4,  -1,   0,  -1,  // 30–39  (36=Kick, 38=Snare head)
        0,  -1,   5,   3,   5,   2,   5,   2,   1,   7,  // 40–49  (40=Snare rim, 42=HiHat closed, 43=Tom3 head, 44=HiHat pedal, 45=Tom2 head, 46=HiHat open, 47=Tom2 rim, 48=Tom1 head, 49=Crash1 head)
        1,   6,   8,   6,  -1,   7,  -1,   8,   3,   6,  // 50–59  (50=Tom1 rim, 51=Ride head, 52=Crash2 head, 53=Ride bell, 55=Crash1 edge, 57=Crash2 edge, 58=Tom3 rim, 59=Ride rim)
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 60–69
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 70–79
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 80–89
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 90–99
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 100–109
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  // 110–119
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1              // 120–127
};

void handleNoteOn(byte channel, byte pitch, byte velocity) {

  dirty = true;
  int8_t note = DRUM_MAP[pitch];
  uint8_t value = (velocity / 127.) * 192;

//  if (note != -1) {
//    notes[note] = value;
//  }

  for (int i = 0; i < 9; i++) {
    notes[i] = value;
  }

  Serial.printf("Ch:%d Note:%d (%d) Vel:%d (%d)\r\n", channel, pitch, note, velocity, value);
}

void setup() {
  Serial.begin(115200);

  MidiSerial.begin(31250, SERIAL_8N1, MIDI_RX_PIN, MIDI_TX_PIN);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.begin(MIDI_CHANNEL_OMNI);
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

    for (int i = 0; i < 9; i++) {
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
