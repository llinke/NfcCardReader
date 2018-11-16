#define ENABLE_SERIAL_DEBUG

// **************************************************
// *** Compiler Flags
// **************************************************
#pragma region Compiler Flags
// --- DEBUG ----------------------------------------
// #define DEBUG_LOOP
// --- WiFi ----------------------------------------
// #define OFFLINE_MODE
// --- MP3 Player -----------------------------------
#define MP3_PLAYER
#ifdef MP3_PLAYER
#define MP3_SERIAL_RX D6
#define MP3_SERIAL_TX D5
#define MP3_BUSY_PIN D7
#endif
#pragma endregion
// **************************************************

// **************************************************
// *** Includes
// **************************************************
#pragma region Includes
#include <Arduino.h>
#include "SerialDebug.h"
#include <vector>
#include <map>
#include <string.h>

#include <Wire.h>
#include "ModFastLed.h"
#include "ModI2C.h"
#include "ModNfc.h"
#include "ModMP3.h"
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
#pragma endregion
// **************************************************

// **************************************************
// *** Application Setup
// **************************************************
#pragma region Application Setup
void setup(void)
{
  Serial.begin(115200);

  DEBUG_PRINTLN("BOOT/SETUP ------------------------------------------------");

  InitFastLED();

  InitI2C();

  InitNfc();

#ifdef MP3_PLAYER
  InitMp3Player();
#endif
}
#pragma endregion
// **************************************************

// **************************************************
// *** Application Loop
// **************************************************
#pragma region Application Loop
void loop(void)
{
  static int nextNfcDebugOutput = millis();

  HandleNfcTag();

  if (nfcNextState != nfcStateUnchanged)
  {
    bool nfcStateChanged = nfcCurrentState != nfcNextState;
    nfcCurrentState = nfcNextState;
    nfcNextState = nfcStateUnchanged;

    if (nfcStateChanged)
    {
      SetColors(activeGrpNr,
                nfcFxCol[nfcCurrentState]);
      SetEffect(activeGrpNr,
                nfcFxNr[nfcCurrentState],
                true,                       // start FX
                false,                      // NOT only once
                direction::FORWARD,         // forward
                -1,                         // no glitter
                nfcFxFps[nfcCurrentState]); // different FPS

      PlayNextMp3();
    }
  }

  if (ledsStarted)
  {
    bool ledsUpdated = false;

    for (int grpNr = 0; grpNr < groupSizes.size(); grpNr++)
    {
      NeoGroup *neoGroup = &(neoGroups.at(grpNr));

      if ((neoGroup->LedFirstNr + neoGroup->LedCount) <= PIXEL_COUNT)
      {
        ledsUpdated |= neoGroup->Update();
      }
    }

    if (ledsUpdated)
    {
#ifdef DEBUG_LOOP
      DEBUG_PRINTLN("Loop: Refreshing LEDs.");
#endif
      FastLED.show();
    }
  }

#ifdef MP3_PLAYER
  RunMp3PlayerLoop();

  if (sfxWatchdogEnabled)
  {
    CheckSfxWatchdog();
  }

  // PlayNextMp3();
#endif

  // REQUIRED: allow processing of interrupts etc
  delay(0);
  //yield();
}
#pragma endregion
// **************************************************
