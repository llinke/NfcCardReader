#define ENABLE_SERIAL_DEBUG

// **************************************************
// *** Compiler Flags
// **************************************************
#pragma region Compiler Flags
// --- DEBUG ----------------------------------------
// #define DEBUG_LOOP
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
#include "ModWifi.h"
#include "ModI2C.h"
#include "ModNfc.h"
#include "ModMP3.h"

#include "src/IFTTT/ESP8266IFTTT.h"
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
#define IFTTT_API_Key "bRrM0DhsRPa10MZi8yjK-V"
#define IFTTT_Event_Name "nfc_card"
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

  InitWifi();

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
        PlayMp3(sfxFolder2018, 0); // ERROR
      }
      else if (nfcCurrentState == nfcStateSuccess)
      {
        uint8_t userAudioTrack = 0;
        uint8_t sfxFolder = sfxFolder2018;
        String userName = "";
        bool userIsKnown = true;

        /* --- 2018 ---------------------------------------- */
        if (uid == "04 92 09 EA 5E 5C 84") // Colin
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 1;
          userName = "Colin";
        }
        else if (uid == "04 8E 08 EA 5E 5C 84") // Jana
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 2;
          userName = "Jana";
        }
        else if (uid == "04 9F 04 EA 5E 5C 84") // Jonah
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 3;
          userName = "Jonah";
        }
        else if (uid == "04 87 07 EA 5E 5C 84") // Kjell
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 4;
          userName = "Kjell";
        }
        else if (uid == "04 8C 05 EA 5E 5C 84") // Noah
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 5;
          userName = "Noah";
        }
        else if (uid == "04 A6 04 EA 5E 5C 84") // Oskar
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 6;
          userName = "Oskar";
        }
        else if (uid == "04 94 04 EA 5E 5C 84") // Paul
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 7;
          userName = "Paul";
        }
        else if (uid == "04 96 07 EA 5E 5C 84") // Per
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 8;
          userName = "Per";
        }
        else if (uid == "04 90 03 EA 5E 5C 84") // Rafael
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 9;
          userName = "Rafael";
        }
        else if (uid == "04 08 FE EA 5E 5C 81") // Test Handle
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 8;
          userName = "Neo"; // Play for Per
        }
        /* --- 2019 ---------------------------------------- */
        else if (uid == "04 7C 9D C2 A8 64 81") // Colin
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 1;
          userName = "Colin";
        }
        else if (uid == "04 74 9D C2 A8 64 81") // Jana
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 2;
          userName = "Jana";
        }
        else if (uid == "04 6C 9D C2 A8 64 81") // Jonah
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 3;
          userName = "Jonah";
        }
        else if (uid == "04 64 9D C2 A8 64 81") // Kjell
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 4;
          userName = "Kjell";
        }
        else if (uid == "04 5C 9D C2 A8 64 81") // Noah
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 5;
          userName = "Noah";
        }
        else if (uid == "04 54 9D C2 A8 64 81") // Oskar
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 6;
          userName = "Oskar";
        }
        else if (uid == "04 4C 9D C2 A8 64 81") // Paul
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 7;
          userName = "Paul";
        }
        else if (uid == "04 44 9D C2 A8 64 81") // Per
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 8;
          userName = "Per";
        }
        else if (uid == "04 3C 9D C2 A8 64 81") // Nils
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 9;
          userName = "Rafael";
        }
        else if (uid == "04 89 98 C2 A8 64 80") // Test Handle
        {
          sfxFolder = sfxFolder2019;
          userAudioTrack = 8;
          userName = "Neo"; // Play for Per
        }
        /* --- ERROR ---------------------------------------- */
        else
        {
          sfxFolder = sfxFolder2018;
          userAudioTrack = 0;
          userName = ""; // Unknown UID
          userIsKnown = false;
        }

        DEBUG_PRINTLN("NFC CARD: UID " + uid + ", playing MP3 #" + String(userAudioTrack));
        PlayMp3(sfxFolder, userAudioTrack);

        if (userIsKnown)
        {
          DEBUG_PRINTLN("NFC CARD: Sending IFTTT event for user: " + uid + ", name: " + userName);
          IFTTT.trigger(IFTTT_Event_Name, IFTTT_API_Key, uid.c_str(), userName.c_str(), "HelloWorld!");
        }
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
