#ifndef __MOD_MP3_H__
#define __MOD_MP3_H__

// **************************************************
// *** Compiler Flags
// **************************************************
#pragma region Compiler Flags
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
#ifdef MP3_PLAYER
#include <Arduino.h>
#include "SerialDebug.h"
// #include "DFMp3Player.cpp"
#include <SoftwareSerial.h>
#include "DFMiniMp3.h"
#endif
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
#ifdef MP3_PLAYER
// SFX to play on initialization
const uint8_t sfxFolder = 1;
volatile uint8_t sfxFilesCount = 0;

volatile int sfxBusyReleasedAt = millis();
volatile int sfxBusyReleasedDelay = 2 * 1000;
volatile bool sfxWatchdogEnabled = false;
volatile int sfxWatchdogCheckAt = 0;
const int sfxWatchdogDelay = 1000;
const uint8_t sfxVolume = 30;
#endif
#pragma endregion
// **************************************************

// **************************************************
// *** DFMP3Player Helper Methods
// **************************************************
#pragma region DFMP3Player Helper Methods
bool IsPlayingMP3()
{
    bool playerIsBusy = !digitalRead(MP3_BUSY_PIN);

    if (playerIsBusy)
    {
        DEBUG_PRINTLN("MP3: IsPlayingMP3 >> YES!");
        return true;
    }

    DEBUG_PRINTLN("MP3: IsPlayingMP3 >> NO!");
    return false;
}

class DfPlayerNotify
{
  public:
    static void OnError(uint16_t errorCode)
    {
        DEBUG_PRINTLN("MP3: COM ERROR " + String(errorCode));
    }
    static void OnPlayFinished(uint16_t track)
    {
        DEBUG_PRINTLN("MP3: Play finished for #" + String(track));
        // RequestNextSfx();
    }
    static void OnCardOnline(uint16_t code)
    {
        DEBUG_PRINTLN("MP3: Card online.");
    }
    static void OnCardInserted(uint16_t code)
    {
        DEBUG_PRINTLN("MP3: Card inserted.");
    }
    static void OnCardRemoved(uint16_t code)
    {
        DEBUG_PRINTLN("MP3: Card removed.");
    }
};

SoftwareSerial dfSoftSerial(MP3_SERIAL_RX, MP3_SERIAL_TX); // RX, TX
DFMiniMp3<SoftwareSerial, DfPlayerNotify> dfPlayer(dfSoftSerial);

void CheckSfxWatchdog()
{
    if (!sfxWatchdogEnabled)
        return;
    if (sfxWatchdogCheckAt == 0 || millis() < sfxWatchdogCheckAt)
        return;

    sfxWatchdogCheckAt = 0;
    sfxWatchdogEnabled = false;

    bool playerIsBusy = !digitalRead(MP3_BUSY_PIN);
    if (playerIsBusy)
    {
        DEBUG_PRINTLN("MP3: Watchdog >> player active, OK.");
        return;
    }

    // DEBUG_PRINTLN("MP3: Watchdog >> not playing, requesting new track.");
    // RequestNextSfx();
}

void PlayNextMp3()
{
    if (IsPlayingMP3())
    {
        DEBUG_PRINTLN("MP3: PlayNextMp3 BLOCKED, cannot play next track.");
        return;
    }

    DEBUG_PRINTLN("MP3: Randomly selecting next MP3 to play for mode...");
    uint8_t nextFolder = sfxFolder;
    uint8_t nextTrack = 1;
    static int sfxFilesCount = 0;
    if (sfxFilesCount == 0)
    {
        sfxFilesCount = dfPlayer.getFolderTrackCount(nextFolder);
        DEBUG_PRINTLN("MP3: Number of files in folder " + String(nextFolder) + ": " + String(sfxFilesCount));
    }
    nextTrack = sfxFilesCount == 0 ? -1 : random(0, sfxFilesCount);

    if (nextTrack >= 0)
    {
        DEBUG_PRINTLN("MP3: Playing folder " + String(nextFolder) + ", track " + String(1 + nextTrack) + "...");
        dfPlayer.playFolderTrack(nextFolder, 1 + nextTrack);

        uint16_t currTrack = dfPlayer.getCurrentTrack();
        DEBUG_PRINTLN("MP3: Track playing " + String(currTrack));
    }

    DEBUG_PRINTLN("MP3: Getting status...");
    uint16_t status = dfPlayer.getStatus();
    DEBUG_PRINTLN("MP3: Status " + String(status));

    // if ((status & 0x0001) != 0x001)
    // {
    // 	DEBUG_PRINTLN("MP3: Not playing, requesting new track...");
    // 	RequestNextSfx();
    // }

    sfxWatchdogCheckAt = millis() + sfxWatchdogDelay;
    sfxWatchdogEnabled = true;
}

void onDfPlayerBusyReleased()
{
    if (millis() < (sfxBusyReleasedAt + sfxBusyReleasedDelay))
    {
        DEBUG_PRINTLN("MP3: IGNORING release of BUSY signal.");
        return;
    }

    DEBUG_PRINTLN("MP3: BUSY signal released...");
    sfxBusyReleasedAt = millis();

    // RequestNextSfx();
}

void InitMp3Player()
{
    pinMode(MP3_BUSY_PIN, INPUT_PULLUP);
    attachInterrupt(MP3_BUSY_PIN, onDfPlayerBusyReleased, RISING);

    delay(1000);

    DEBUG_PRINTLN("MP3: Configuring SoftSerial...");
    dfSoftSerial.begin(9600);
    dfSoftSerial.setTimeout(600);

    DEBUG_PRINTLN("MP3: Initializing...");
    dfPlayer.begin();
    DEBUG_PRINTLN("MP3: Resetting...");
    dfPlayer.reset();

    delay(1000);

    DEBUG_PRINTLN("MP3: Getting status...");
    uint16_t status = dfPlayer.getStatus();
    DEBUG_PRINTLN("MP3: Status " + String(status));

    DEBUG_PRINTLN("MP3: Changing volume...");
    dfPlayer.setVolume(sfxVolume);
    uint16_t volume = dfPlayer.getVolume();
    DEBUG_PRINTLN("MP3: Volume " + String(volume));

    DEBUG_PRINTLN("MP3: Getting number of MP3s...");
    uint16_t count = dfPlayer.getTotalTrackCount();
    DEBUG_PRINTLN("MP3: Files " + String(count));

    DEBUG_PRINTLN("MP3: Setting repeat mode...");
    dfPlayer.setRepeatPlay(false);
    uint16_t mode = dfPlayer.getPlaybackMode();
    DEBUG_PRINTLN("MP3: Playback mode " + String(mode));

    DEBUG_PRINTLN("MP3: Setting equalizer...");
    dfPlayer.setEq(DfMp3_Eq_Normal);
    DfMp3_Eq eqmode = dfPlayer.getEq();
    DEBUG_PRINTLN("MP3: EQ " + String(eqmode));

    // PlayNextMp3();

    DEBUG_PRINTLN("MP3: Setup finished!");
}

void RunMp3PlayerLoop()
{
    // calling dfPlayer.loop() periodically allows for notifications
    // to be handled without interrupts
    dfPlayer.loop();
}
#pragma endregion
// **************************************************

#endif