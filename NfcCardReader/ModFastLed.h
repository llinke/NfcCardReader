#ifndef __MOD_FASTLED_H__
#define __MOD_FASTLED_H__

// **************************************************
// *** Compiler Flags
// **************************************************
#pragma region Compiler Flags
// --- Demo --------- -------------------------------
#define PLAY_DEMO true
// --- FX/Colors ------------------------------------
//#define DO_NOT_START_FX_ON_INIT
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

#include "FastLedInclude.h"
#include "ColorPalettes.h"
#include "NeoGroup.cpp"
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
// [FastLED]
// Static size
struct CRGB leds[PIXEL_COUNT];
// Dynamic size:
//struct CRGB *leds = NULL;

bool ledsInitialized = false;
bool ledsStarted = false;
volatile uint8_t globalBrightness = 128;

// [NeoGroup]
const std::vector<int> groupSizes = {PIXEL_COUNT};
const std::vector<int> groupOffsets = {1};
volatile int activeGrpNr = 0;

// 1: Wave, 2: Dynamic Wave, 3: Noise, 4: Confetti, 5: Fade, 6: Comet, 7: Orbit, 8: Fill
const uint8_t fxNrWave = 1;
const uint8_t fxNrDynamicWave = 2;
const uint8_t fxNrNoise = 3;
const uint8_t fxNrConfetti = 4;
const uint8_t fxNrFade = 5;
const uint8_t fxNrComet = 6;
const uint8_t fxNrOrbit = 7;
const uint8_t fxNrFill = 8;
const int maxFxNr = 7;
const int defaultFxNr = fxNrWave; //fxNrFade;
std::vector<int> currFxNr;
int maxColNr = 1;           // will be dynamically assigned once palettes are generated
const int defaultColNr = 1; // "NFC Idle"
std::vector<int> currColNr;
const int defaultFps = 75; //25;
std::vector<int> currFps;
const int defaultGlitter = 0; //32;
std::vector<int> currGlitter;
uint8_t currentHue = HUE_AQUA;
uint8_t currentSat = 255;

//std::vector<NeoGroup *> neoGroups;
std::vector<NeoGroup> neoGroups;
#pragma endregion
// **************************************************

// **************************************************
// *** FastLED Helper methods
// **************************************************
#pragma region FastLED Helper methods
int startStrip()
{
    if (!ledsInitialized)
        return -1;

    ledsStarted = true;
    return PIXEL_COUNT;
}

int stopStrip()
{
    ledsStarted = false;

    for (int i = 0; i < neoGroups.size(); i++)
    {
        NeoGroup *neoGroup = &(neoGroups.at(i));
        neoGroup->Stop(true);
    }

    FastLED.clear(true);
    FastLED.show();

    return 0;
}

int addGroup(String grpId, int ledFirst, int ledCount, int ledOffset)
{
    if ((ledFirst >= PIXEL_COUNT) ||
        (ledCount <= 0) ||
        (ledFirst + ledCount) > PIXEL_COUNT)
        return -((((3 * 1000) + ledFirst) * 1000) + ledCount); // Invalid parameter

    DEBUG_PRINTLN("Adding group '" + String(grpId) + "' at " + String(ledFirst) + " with size " + String(ledCount) + ".");

    // V1: new NeoGroup
    //NeoGroup *newGroup = new NeoGroup(grpId, ledFirst, ledCount, ledOffset);
    //neoGroups.push_back(newGroup);
    // V2: NeoGroup w/o new
    NeoGroup newGroup = NeoGroup(grpId, ledFirst, ledCount, ledOffset);
    neoGroups.push_back(newGroup);

    currFxNr.push_back(defaultFxNr);
    currColNr.push_back(defaultColNr);
    currFps.push_back(defaultFps);
    currGlitter.push_back(defaultGlitter);

    DEBUG_PRINTLN("GroupCount: " + String(neoGroups.size()) + ".");

    return neoGroups.size();
}

int initStrip(bool doStart = false, bool playDemo = true)
{
    if (ledsInitialized)
    {
        return doStart ? startStrip() : PIXEL_COUNT;
    }

    DEBUG_PRINTLN("LEDStrip --------------------------------------------------");
    /*	
	DEBUG_PRINTLN("Allocating memory for LED strip data.");
	leds = (struct CRGB *)malloc(PIXEL_COUNT * sizeof(struct CRGB));
	*/
    DEBUG_PRINTLN("Assigning LEDs to FastLED.");
    FastLED.addLeds<PIXEL_TYPE, PIXEL_PIN>(leds, PIXEL_COUNT);
    //FastLED.setMaxPowerInVoltsAndMilliamps(5,3000);
    FastLED.setBrightness(globalBrightness);
    FastLED.setDither(0);
    FastLED.clear(true);
    FastLED.show();

    ledsInitialized = true;

    if (playDemo)
    {
        DEBUG_PRINT("Playing demo effect...");
        CRGBPalette16 colorPalette = NeoGroup::GenerateRGBPalette(ColorPalettes.find("Regenbogen")->second);
        for (int dot = 0; dot < 256; dot++)
        {
            /*
      // Comet effect :-)
      fadeToBlackBy(leds, PIXEL_COUNT, 8);
      int variant = (PIXEL_COUNT / 16);
      int pos = ease8InOutQuad(dot) + random(0 - variant, 0 + variant);
      pos = (pos * PIXEL_COUNT) / 256;
      pos = constrain(pos, 0, PIXEL_COUNT);
      //DEBUG_PRINT("Setting pixel #");
      //DEBUG_PRINTLN(pos);
      int bright = random(64, 255);

      uint8_t colpos = dot + random8(16) - 8;
      nblend(leds[pos], ColorFromPalette(colorPalette, colpos, bright), 128);
      */

            int bright = dot < 32 ? dot * 8 : 255;
            fill_solid(leds, PIXEL_COUNT, ColorFromPalette(colorPalette, dot, bright));
            FastLED.show();
            delay(5);
            DEBUG_PRINT(".");
        }
        DEBUG_PRINTLN("DONE");

        DEBUG_PRINTLN("Fading away demo effect.");
        for (int fade = 0; fade < 20; fade++)
        {
            fadeToBlackBy(leds, PIXEL_COUNT, 20);
            FastLED.show();
            delay(50);
        }
        DEBUG_PRINTLN("Clearing LEDs.");
        FastLED.clear(true);
        FastLED.show();
    }

    neoGroups.clear();
    // DEBUG_PRINTLN("Adding special groups.");
    // // Group 0: all LEDs
    // addGroup("All LEDs' group", 0, PIXEL_COUNT, 0);
    // activeGrpNr = 0;

    DEBUG_PRINTLN("Adding " + String(groupSizes.size()) + " groups.");
    int nextGroupStart = 0;
    for (int i = 0; i < groupSizes.size(); i++)
    {
        String groupName = "Group " + String(i + 1);
        addGroup(groupName, nextGroupStart, groupSizes[i], groupOffsets[i]);
        nextGroupStart += groupSizes[i];
    }
    activeGrpNr = 0;

    return doStart ? startStrip() : PIXEL_COUNT;
}

bool isGroupActive(int grpNr)
{
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    return neoGroup->Active;
}

bool isGroupFadingOut(int grpNr)
{
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    return !(neoGroup->Active) && (neoGroup->IsFadingOut());
}

int startGroup(int grpNr, bool runOnlyOnce = false)
{
    DEBUG_PRINTLN("Starting group #" + String(grpNr) + "...");
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    neoGroup->Start(runOnlyOnce);

    return grpNr;
}

int stopGroup(int grpNr, bool stopNow = false)
{
    DEBUG_PRINTLN("Stopping group #" + String(grpNr) + "...");
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    neoGroup->Stop(stopNow);

    return grpNr;
}

int setGrpEffect(
    int grpNr,
    pattern pattern,
    int amountglitter = -1,
    int fps = 0,
    direction direction = FORWARD,
    mirror mirror = NOMIRROR,
    wave wave = LINEAR,
    int speed = 1,
    double fpsFactor = 1.0)
{
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    neoGroup->Stop(true);

    //int fxGlitter = amountglitter <= 0 ? neoGroup->GetGlitter() : amountglitter;
    int fxGlitter = amountglitter <= 0 ? currGlitter.at(grpNr) : amountglitter;
    //int fxFps = fps <= 0 ? neoGroup->GetFps() : fps;
    int fxFps = fps > 0 ? fps : currFps.at(grpNr);

    uint16_t result = neoGroup->ConfigureEffect(pattern, fxGlitter, fxFps, direction, mirror, wave, speed, fpsFactor);
    //neoGroup->Start();

    return result;
}

int setGrpColors(
    int grpNr,
    std::vector<CRGB> colors,
    bool clearFirst = true,
    bool generatePalette = true,
    bool crossFade = false)
{
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    //neoGroup->Stop();
    uint16_t result = neoGroup->ConfigureColors(colors, clearFirst, generatePalette, crossFade);
    //neoGroup->Start();

    return result;
}

// [Event handling helper methods]
void StartStopEffect(int grpNr)
{
    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    DEBUG_PRINTLN("StartStopEffect ---------------------------------------------");
    DEBUG_PRINTLN("Fx: Starting/stopping group #" + String(grpNr) + ": " + String((neoGroup->Active)) + "->" + String(!(neoGroup->Active)) + ".");
    bool stopNow = false;
    if (neoGroup->Active)
        stopGroup(grpNr, stopNow);
    else
        startGroup(grpNr);
}

void SetEffect(int grpNr, int fxNr,
               bool startFx, bool onlyOnce,
               direction fxDirection = direction::FORWARD,
               int amountGlitter = -1,
               int targetFps = 0,
               int speed = 1)
{
    DEBUG_PRINTLN("SetEffect ---------------------------------------------------");
    DEBUG_PRINTLN("Fx: Configuring LED effect #" + String(fxNr) + " for group #" + String(grpNr) + ".");

    if (fxNr == 0)
    {
        DEBUG_PRINTLN("Fx: Choosing random effect.");
        SetEffect(grpNr, random8(1, maxFxNr), startFx, onlyOnce);
        return;
    }

    String fxPatternName = "";
    pattern fxPattern = pattern::STATIC;
    int fxGlitter = currGlitter.at(grpNr);
    int fxFps = currFps.at(grpNr);
    double fxFpsFactor = 1.0;
    mirror fxMirror = NOMIRROR;
    wave fxWave = wave::LINEAR;
    int fxSpeed = speed;

    NeoGroup *neoGroup = &(neoGroups.at(grpNr));
    switch (fxNr)
    {
    case fxNrWave:
        fxPatternName = "Wave";
        fxPattern = pattern::WAVE;
        fxDirection = direction::REVERSE;
        fxMirror = mirror::MIRRORODDEVEN;
        fxFpsFactor = 0.5; // half FPS looks better
        break;
    case fxNrDynamicWave:
        fxPatternName = "Dynamic Wave";
        fxPattern = pattern::DYNAMICWAVE;
        fxMirror = mirror::MIRRORODDEVEN; // mirror::NOMIRROR;
        // fxFpsFactor = 0.5; // half FPS looks better
        break;
    case fxNrNoise:
        fxPatternName = "Noise";
        fxPattern = pattern::NOISE;
        fxMirror = mirror::MIRRORODDEVEN; // mirror::NOMIRROR;
        // fxFps *= 2; // double FPS looks better
        fxFpsFactor = 2.0; // double FPS looks better
        break;
    case fxNrConfetti:
        fxPatternName = "Confetti";
        fxPattern = pattern::CONFETTI;
        fxGlitter = 0;
        // fxFps /= 2; // half FPS looks better
        fxFpsFactor = 0.5; // half FPS looks better
        break;
    case fxNrFade:
        fxPatternName = "Fade";
        fxPattern = pattern::FADE;
        // fxFps /= 2; // half FPS looks better
        fxFpsFactor = 0.5; // half FPS looks better
        break;
    case fxNrComet:
        fxPatternName = "Comet";
        fxPattern = pattern::COMET;
        fxWave = wave::EASEINOUT;
        // fxWave = wave::SINUS;
        // fxFps *= 3; //1.5; // faster FPS looks better
        // fxFpsFactor = 1.5; // faster FPS looks better
        fxMirror = mirror::MIRRORODDEVEN;
        break;
    case fxNrOrbit:
        fxPatternName = "Orbit";
        fxPattern = pattern::COMET;
        // fxWave = wave::EASEINOUT;
        fxWave = wave::SINUS;
        // fxFps *= 1.5; // faster FPS looks better
        // fxFpsFactor = 1.5; // faster FPS looks better
        fxFpsFactor = 0.5; // half FPS looks better
        fxMirror = mirror::NOMIRROR;
        // fxMirror = mirror::MIRRORODDEVEN;
        break;
    case fxNrFill:
        fxPatternName = "Fill";
        fxPattern = pattern::FILL;
        fxWave = wave::EASEINOUT;
        // fxFps *= 1.5; // faster FPS looks better
        fxFpsFactor = 1.5; // faster FPS looks better
        // fxMirror = mirror::NOMIRROR;
        fxMirror = mirror::MIRRORODDEVEN;
        break;
    default:
        fxPatternName = "Static";
        fxPattern = pattern::STATIC;
        fxMirror = mirror::NOMIRROR;
        break;
    }

    DEBUG_PRINTLN("Fx: Changing effect to '" + String(fxPatternName) + "'.");
    if (targetFps > 0)
        fxFps = targetFps;
    setGrpEffect(
        grpNr,
        fxPattern,
        fxGlitter,
        fxFps,
        fxDirection,
        fxMirror,
        fxWave,
        fxSpeed,
        fxFpsFactor);

    if (startFx)
    {
        startGroup(grpNr, onlyOnce);
    }
    DEBUG_PRINTLN("SetEffect ---------------------------------------------------");
}

void InitColorNames()
{
    InitColorPalettes();
    /*
	for (std::map<String, std::vector<CRGB>>::const_iterator it = ColorPalettes.begin(); it != ColorPalettes.end(); ++it)
	{
		String key = it->first;
		ColorNames.push_back(key);
	}
	*/
}

void SetColors(int grpNr, String palKey)
{
    DEBUG_PRINTLN("Col: Changing color palette to '" + palKey + "'.");
    if (ColorPalettes.find(palKey) != ColorPalettes.end())
    {
        std::vector<CRGB> colors = ColorPalettes.find(palKey)->second;
        if (colors == (std::vector<CRGB>)NULL)
        {
            DEBUG_PRINTLN("Col: Generating colours for hue " + String(currentHue) + ", sat " + String(currentSat) + " using method '" + palKey + "'.");
            colors = GeneratePaletteFromHue(palKey, currentHue, currentSat);
        }
        setGrpColors(grpNr, colors, true, true, CROSSFADE_PALETTES);
    }
    else
    {
        DEBUG_PRINTLN("Col: ERROR: Color palette '" + palKey + "' not found!");
    }
}

void SetColors(int grpNr, int colNr)
{
    DEBUG_PRINTLN("SetColors --------------------------------------------------");
    DEBUG_PRINTLN("Col: Configuring LED colors #" + String(colNr) + " for group #" + String(grpNr) + ".");

    if (colNr == 0)
    {
        DEBUG_PRINTLN("Col: Choosing random color palette.");
        SetColors(grpNr, random8(1, maxColNr));
        return;
    }

    String palKey = ColorNames[colNr - 1];
    SetColors(grpNr, palKey);
}

void NextEffect(int nextFx = -1)
{
    if (nextFx < 0)
    {
        (currFxNr.at(activeGrpNr))++;
    }
    else
    {
        currFxNr.at(activeGrpNr) = nextFx;
    }
    if (currFxNr.at(activeGrpNr) > maxFxNr)
    {
        currFxNr.at(activeGrpNr) = 1;
    }
    DEBUG_PRINTLN("CONTROL: Changing effect number to: " + String(currFxNr.at(activeGrpNr)) + ".");
    SetEffect(activeGrpNr, currFxNr.at(activeGrpNr), true, false);
}

void NextColor(int nextCol = -1)
{
    if (nextCol < 0)
    {
        (currColNr.at(activeGrpNr))++;
    }
    else
    {
        currColNr.at(activeGrpNr) = nextCol;
    }
    if (currColNr.at(activeGrpNr) > maxColNr)
    {
        currColNr.at(activeGrpNr) = 1;
    }
    DEBUG_PRINTLN("CONTROL: Changing color number to: " + String(currColNr.at(activeGrpNr)) + ".");
    SetColors(activeGrpNr, currColNr.at(activeGrpNr));
}

void InitFastLED()
{
    DEBUG_PRINTLN("FastLED: Initializing color palettes.");
    InitColorNames();
    maxColNr = ColorNames.size();

    DEBUG_PRINTLN("FastLED: Initializing LED strip.");
    initStrip(true, PLAY_DEMO);
    DEBUG_PRINTLN("FastLED: Amount of LEDs: " + String(PIXEL_COUNT) + ".");

    DEBUG_PRINTLN("FastLED: Starting LED strip.");
    startStrip();

    for (int grpNr = 0; grpNr < groupSizes.size(); grpNr++)
    {
        DEBUG_PRINTLN("FastLED: Setting up and starting group #" + String(grpNr) + ".");
        bool startFx = true;
#ifdef DO_NOT_START_FX_ON_INIT
        startFx = false;
#endif
        int currCol = defaultColNr;
        currColNr[grpNr] = currCol;
        SetColors(grpNr, currCol);

        int currFx = defaultFxNr;
        currFxNr[grpNr] = currFx;
        SetEffect(grpNr, currFx, startFx, false);
        //startGroup(grpNr);
    }
    //activeGrpNr = 0;

    DEBUG_PRINTLN("FastLED: Active group #" + String(activeGrpNr));
}

#pragma endregion
// **************************************************

#endif