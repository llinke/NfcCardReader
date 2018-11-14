#define ENABLE_SERIAL_DEBUG

// **************************************************
// *** Compiler Flags
// **************************************************
#pragma region Compiler Flags
// --- DEBUG ----------------------------------------
// #define DEBUG_LOOP
// --- Demo --------- -------------------------------
#define PLAY_DEMO true
// --- FX/Colors ------------------------------------
//#define DO_NOT_START_FX_ON_INIT
// --- WiFi ----------------------------------------
// #define OFFLINE_MODE
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

#include <Wire.h>
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"
#include "src/NDEF/NfcAdapter.h"
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
const std::vector<int> groupOffsets = {0};
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

// [I2C]
const int I2C_SCL_PIN = D1;
const int I2C_SCA_PIN = D2;
const int I2C_BUS_SPEED = 100000; // 100kHz for PCF8574
const int I2C_CLK_STRETCH_LIMIT = 1600;

// [NFC]
const int NfcTagPresentTimeout = 0; //100;
// const int NfcScanInterval = 1000;

volatile unsigned long nextScan = millis();
volatile bool isNfcCardDetected = false;

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// [NFC State Machine]
const uint8_t nfcStateUnchanged = -1;
const uint8_t nfcStateIdle = 0;
const uint8_t nfcStateSuccess = 1;
const uint8_t nfcStateError = 2;
const std::vector<uint8_t> nfcFxNr = {fxNrWave, fxNrWave, fxNrFade};
const std::vector<String> nfcFxCol = {"NfcIdle", "NfcSuccess", "NfcError"};
const std::vector<int> nfcFxFps = {75, 75, 100};
const std::vector<int> nfcScanInterval = {1000, 8000, 8000};

volatile uint8_t nfcCurrentState = nfcStateIdle;
volatile uint8_t nfcNextState = nfcStateUnchanged;
#pragma endregion
// **************************************************

// **************************************************
// *** Event Handlers
// **************************************************
#pragma region Event Handlers
#pragma endregion
// **************************************************

// **************************************************
// *** FastLED Helper methods
// **************************************************
#pragma region FastLED Helper methods
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

#ifndef OFFLINE_MODE
  //InitWifi(false); // do not use timeout, wait at WiFi config if not connected
#endif

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

// **************************************************
// *** I2C Helper Methods
// **************************************************
#pragma region I2C Helper Methods
void InitI2C()
{
  DEBUG_PRINTLN("I2C: Initializing Wire");
  Wire.begin(I2C_SCA_PIN, I2C_SCL_PIN);

  DEBUG_PRINTLN("I2C: setting bus speed to " + String(I2C_BUS_SPEED) + ".");
  Wire.setClock(I2C_BUS_SPEED);

  DEBUG_PRINTLN("I2C: setting clock stretch limit to " + String(I2C_CLK_STRETCH_LIMIT) + ".");
  Wire.setClockStretchLimit(I2C_CLK_STRETCH_LIMIT);

  ScanI2C();
}

void ScanI2C()
{
  byte error, address;
  int nDevices;

  DEBUG_PRINTLN("I2C: scanning I2C Bus...");
  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      DEBUG_PRINT("I2C: device found at address 0x");
      if (address < 16)
      {
        DEBUG_PRINT("0");
      }
      DEBUG_PRINT(address, HEX);
      DEBUG_PRINTLN(" !");

      nDevices++;
    }
    else if (error == 4)
    {
      DEBUG_PRINT("I2C: Unknown error at address 0x");
      if (address < 16)
      {
        DEBUG_PRINT("0");
      }
      DEBUG_PRINTLN(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    DEBUG_PRINTLN("I2C: No I2C devices found\n");
  }
  else
  {
    DEBUG_PRINTLN("I2C: Done.\n");
  }

  delay(2000);
}
#pragma endregion
// **************************************************

// **************************************************
// *** I2C Helper Methods
// **************************************************
#pragma region NFC Helper Methods
void InitNfc()
{
  DEBUG_PRINTLN("NFC: Starting NDEF Reader");

  nfc.begin();
}

void HandleNfcTag()
{
  static int count = 0;

  if (millis() < nextScan)
    return;

  count++;
  DEBUG_PRINTLN("\nNFC: scanning NFC tag (#" + String(count) + ")");

  isNfcCardDetected = nfc.tagPresent(NfcTagPresentTimeout);
  DEBUG_PRINTLN("NFC: NFC tag detected:" + String(isNfcCardDetected));

  if (!isNfcCardDetected)
  {
    if (nfcCurrentState != nfcStateIdle)
    {
      nfcNextState = nfcStateIdle;
    }
    nextScan = millis() + nfcScanInterval[nfcNextState != nfcStateUnchanged ? nfcNextState : nfcCurrentState];
    DEBUG_PRINTLN("NFC: next scan in " + String(nextScan - millis()) + "ms.");
    return;
  }

  isNfcCardDetected = false; //!nfc.tagPresent(NfcTagPresentTimeout);

  NfcTag tag = nfc.read();
  DEBUG_PRINTLN(tag.getTagType());
  DEBUG_PRINT("NFC: tag read, UID: ");
  DEBUG_PRINTLN(tag.getUidString());

  if (!tag.hasNdefMessage()) // every tag won't have a message
  {
    DEBUG_PRINTLN("NFC: no NDEF message on tag.");
    nfcNextState = nfcStateError;
    nextScan = millis() + nfcScanInterval[nfcNextState];
    DEBUG_PRINTLN("NFC: next scan in " + String(nextScan - millis()) + "ms.");
    return;
  }

  nfcNextState = nfcStateSuccess;
  nextScan = millis() + nfcScanInterval[nfcNextState];
  DEBUG_PRINTLN("NFC: next scan in " + String(nextScan - millis()) + "ms.");

  NdefMessage message = tag.getNdefMessage();
  DEBUG_PRINT("NFC: This NFC Tag contains an NDEF Message with ");
  DEBUG_PRINT(message.getRecordCount());
  DEBUG_PRINT(" NDEF Record");
  if (message.getRecordCount() != 1)
  {
    DEBUG_PRINT("s");
  }
  DEBUG_PRINTLN(".");

  // cycle through the records, printing some info from each
  int recordCount = message.getRecordCount();
  for (int i = 0; i < recordCount; i++)
  {
    DEBUG_PRINT("NFC: NDEF Record ");
    DEBUG_PRINTLN(i + 1);
    NdefRecord record = message.getRecord(i);
    // NdefRecord record = message[i]; // alternate syntax

    DEBUG_PRINT("  TNF: ");
    DEBUG_PRINTLN(record.getTnf());
    DEBUG_PRINT("  Type: ");
    DEBUG_PRINTLN(record.getType()); // will be "" for TNF_EMPTY

    // The TNF and Type should be used to determine how your application processes the payload
    // There's no generic processing for the payload, it's returned as a byte[]
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);

    // Print the Hex and Printable Characters
    DEBUG_PRINT("  Payload (HEX): ");
    PrintHexChar(payload, payloadLength);

    // Force the data into a String (might work depending on the content)
    // Real code should use smarter processing
    String payloadAsString = "";
    for (int c = 0; c < payloadLength; c++)
    {
      payloadAsString += (char)payload[c];
    }
    DEBUG_PRINT("  Payload (as String): ");
    DEBUG_PRINTLN(payloadAsString);

    // id is probably blank and will return ""
    String uid = record.getId();
    if (uid != "")
    {
      DEBUG_PRINT("  ID: ");
      DEBUG_PRINTLN(uid);
    }
  }
}
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

  // REQUIRED: allow processing of interrupts etc
  delay(0);
  //yield();
}
#pragma endregion
// **************************************************
