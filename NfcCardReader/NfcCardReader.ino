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

  static String uid = "";

  uid = HandleNfcTag();

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

      // PlayNextMp3();
      if (nfcCurrentState == nfcStateError)
      {
        PlayMp3(sfxFolder, 0); // ERROR
      }
      else
      {
        uint8_t userAudioTrack = 0;
        String userName = "";
        if (uid == "04 92 09 EA 5E 5C 84") // Colin
        {
          userAudioTrack = 1;
          userName = "Colin";
        }
        else if (uid == "04 8E 08 EA 5E 5C 84") // Jana
        {
          userAudioTrack = 2;
          userName = "Jana";
        }
        else if (uid == "04 9F 04 EA 5E 5C 84") // Jonah
        {
          userAudioTrack = 3;
          userName = "Jonah";
        }
        else if (uid == "04 87 07 EA 5E 5C 84") // Kjell
        {
          userAudioTrack = 4;
          userName = "Kjell";
        }
        else if (uid == "04 8C 05 EA 5E 5C 84") // Noah
        {
          userAudioTrack = 5;
          userName = "Noah";
        }
        else if (uid == "04 A6 04 EA 5E 5C 84") // Oskar
        {
          userAudioTrack = 6;
          userName = "Oskar";
        }
        else if (uid == "04 94 04 EA 5E 5C 84") // Paul
        {
          userAudioTrack = 7;
          userName = "Paul";
        }
        else if (uid == "04 96 07 EA 5E 5C 84") // Per
        {
          userAudioTrack = 8;
          userName = "Per";
        }
        else if (uid == "04 90 03 EA 5E 5C 84") // Rafael
        {
          userAudioTrack = 9;
          userName = "Rafael";
        }
        else if (uid == "04 08 FE EA 5E 5C 81") // Test Handle
        {
          userAudioTrack = 8;
          userName = "Neo"; // Play for Per
        }
        else
        {
          userAudioTrack = 0;
          userName = ""; // Unknown UID
        }

        DEBUG_PRINTLN("NFC: UID " + uid + ", playing MP3 #" + String(userAudioTrack));
        PlayMp3(sfxFolder, userAudioTrack);
      }
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
