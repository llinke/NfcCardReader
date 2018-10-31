#include <Arduino.h>
#include "SerialDebug.h"
#include "FastLedInclude.h"
// #include <ArduinoSTL.h>
#include <vector>
// #include <map>

// **************************************************
// *** Serial Debugging
// **************************************************
#pragma region Serial Debug
// Uncomment to enable printing debug messages.
#ifdef ENABLE_SERIAL_DEBUG
// #define ENABLE_SERIAL_DEBUG_GRP
#endif
#define DEBUG_GRP_PRINTER Serial
#ifdef ENABLE_SERIAL_DEBUG_GRP
#define DEBUG_GRP_PRINT(...)                  \
	{                                         \
		DEBUG_GRP_PRINTER.print(__VA_ARGS__); \
	}
#define DEBUG_GRP_PRINTLN(...)                  \
	{                                           \
		DEBUG_GRP_PRINTER.println(__VA_ARGS__); \
	}
#else
#define DEBUG_GRP_PRINT(...) \
	{                        \
	}
#define DEBUG_GRP_PRINTLN(...) \
	{                          \
	}
#endif
#pragma endregion

#define FIRE_COOLING 55
#define FIRE_SPARKING 120

#define FADEOUT_DURATION 500
#define FADEOUT_STEPS 16
#define CROSSFADE_PALETTES true
#define CROSSFADE_MAX_CHANGE_PER_STEP 16

// #define FxColorWavesWithBrightness

#pragma region Enums
enum pattern
{
	NOFX,
	STATIC,
	FILL,
	FADE,
	WAVE,
	DYNAMICWAVE,
	NOISE,
	RAINBOW,
	CONFETTI,
	FIRE,
	COMET
};

enum wave
{
	LINEAR,
	SINUS,
	/*
	EASEIN,
	EASEOUT,
	*/
	EASEINOUT
};

enum direction
{
	FORWARD,
	REVERSE
};

enum mirror
{
	MIRROR0, // no mirroring
	MIRROR1, // set even/odd as mirror values
	MIRROR2  // mirror each second value
};
#pragma endregion

class NeoGroup
{
#ifndef PIXEL_USE_OFFSET
	CRGB *LedFirst;
#endif
	int LedOffset = 0;

	int fxFps;
	double fxFpsFactor = 1.0;
	unsigned long lastUpdate;
	int fxStep;
	int fxSpeed = 1;

	direction fxDirection;
	wave fxWave;
	int fxFadeOut = 0;
	int fxAmountGlitter;
	uint8_t fxLength;
	mirror fxMirror = MIRROR0;
	bool onlyOnce = false;

	//std::vector<CRGB> currentColors = {};
	bool crossFadeColors = CROSSFADE_PALETTES; //false;
	std::vector<CRGB> currentColors = {0x000000, 0x000000};
	CRGBPalette16 colorPalette = CRGBPalette16();
	CRGBPalette16 colorPaletteNew = CRGBPalette16();
	uint8_t firePaletteNr = 6;

	void (NeoGroup::*effectFunc)();

  public:
	String GroupID;
	int LedFirstNr;
	int LedCount;
	bool Active;
	bool IsFadingOut()
	{
		return fxFadeOut > 0;
	}
	bool IsRunningOnce()
	{
		return onlyOnce;
	}

#pragma region Constructor
	NeoGroup(String groupID, int ledFirst, int ledCount, int ledOffset = 0)
	{
		GroupID = groupID;
		Active = false;
		fxStep = 0;
		fxSpeed = 1;
		LedFirstNr = ledFirst;
#ifndef PIXEL_USE_OFFSET
		LedFirst = &leds[ledFirst];
#endif
		LedCount = ledCount;
		LedOffset = ledOffset;
		colorPalette = GenerateRGBPalette({0x000000, 0x000000});
	}
#pragma endregion

#pragma region NeoGroup functions
	CRGB GetColorFromPaletteAt(uint8_t idx, uint8_t brightness = 255)
	{
		return ColorFromPalette(colorPalette, idx, brightness);
	}

	uint16_t ConfigureEffect(
		pattern pattern,
		int amountglitter = 0,
		int fps = 50,
		direction direction = FORWARD,
		mirror mirror = MIRROR0,
		wave wave = LINEAR,
		int speed = 1,
		double fpsFactor = 1.0)
	{
		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Stopping effect execution.");
		Stop(true);

		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Configuring effect parameters.");
		ChangeFps(fps);
		fxFpsFactor = fpsFactor;
		fxDirection = direction;
		fxStep = (fxDirection == REVERSE) ? 255 : 0;
		fxSpeed = speed;
		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: direction " + fxDirection + ", starting FX at " + String(fxStep) + ".");
		fxWave = wave;
		ChangeGlitter(amountglitter);
		fxMirror = mirror;
		fxLength = 255;

		if (pattern == STATIC)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Static'.");
			effectFunc = &NeoGroup::FxStatic;
		}
		if (pattern == FILL)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Fill'.");
			effectFunc = &NeoGroup::FxFill;
		}
		if (pattern == FADE)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Fade'.");
			effectFunc = &NeoGroup::FxFade;
		}
		if (pattern == WAVE)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Wave'.");
			effectFunc = &NeoGroup::FxWave;
			fxLength = constrain((LedCount * 1.5), 0, 255);
		}
		if (pattern == DYNAMICWAVE)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Dynamic Wave'.");
			effectFunc = &NeoGroup::FxColorWaves;
		}
		if (pattern == NOISE)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Noise'.");
			effectFunc = &NeoGroup::FxNoise;
		}
		if (pattern == RAINBOW)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Rainbow'.");
			effectFunc = &NeoGroup::FxRainbow;
			fxLength = constrain((LedCount * 2), 0, 255);
		}
		if (pattern == CONFETTI)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Confetti'.");
			effectFunc = &NeoGroup::FxConfetti;
			fxAmountGlitter = 0;
		}
		if (pattern == FIRE)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Fire'.");
#ifdef PIXEL_USE_OFFSET
			FillPixels(0, LedCount, 0x000000);
#else
			fill_solid(LedFirst, LedCount, 0x000000);
#endif
			effectFunc = &NeoGroup::FxFire;
			fxAmountGlitter = 0;
		}
		if (pattern == COMET)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgFX: Setting FX 'Comet'.");
#ifdef PIXEL_USE_OFFSET
			FillPixels(0, LedCount, 0x000000);
#else
			fill_solid(LedFirst, LedCount, 0x000000);
#endif
			effectFunc = &NeoGroup::FxComet;
			fxAmountGlitter = 0;
		}
		return 0;
	}

	uint16_t ConfigureColors(
		std::vector<CRGB> colors,
		bool clearFirst = true,
		bool generatePalette = true,
		bool crossFade = CROSSFADE_PALETTES)
	{
		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: Configuring colors.");
		if (clearFirst)
		{
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: Clearing old list of colors.");
			currentColors.clear();
		}

		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: Adding " + String(colors.size()) + " CRGB colors to internal list.");
		for (CRGB color : colors)
		{
			currentColors.push_back(color);
		}

		if (generatePalette)
		{
			if (currentColors.size() != 0)
			{
				crossFadeColors = crossFade;
				if (crossFadeColors)
				{
					DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: Crossfading to new palette.");
					colorPaletteNew = GenerateRGBPalette(currentColors);
				}
				else
				{
					DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: Switching to new palette.");
					colorPalette = GenerateRGBPalette(currentColors);
				}
			}
			else
			{
				DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].CfgColor: No colors, using empty list.");
				if (crossFadeColors)
				{
					colorPaletteNew = GenerateRGBPalette({});
				}
				else
				{
					colorPalette = GenerateRGBPalette({});
				}
			}
		}
		return currentColors.size();
	}

	void ChangeFps(int fps)
	{
		fxFps = fps;
	}

	int GetFps()
	{
		return fxFps;
	}

	void ChangeGlitter(int amountglitter)
	{
		fxAmountGlitter = amountglitter;
	}

	int GetGlitter()
	{
		return fxAmountGlitter;
	}

	void Start(bool runOnlyOnce = false)
	{
		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Start: Starting group.");
		fxStep = (fxDirection == REVERSE) ? 255 : 0;
		onlyOnce = runOnlyOnce;
		if (onlyOnce)
			DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Start: Will run only ONCE.");
		Active = true;
		fxFadeOut = 0;
		lastUpdate = millis();
	}

	void Stop(bool stopNow = false)
	{
		DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].GRP: Stopping group.");
		Active = false;
		fxFadeOut = (stopNow) ? 0 : FADEOUT_STEPS;
		if (stopNow)
		{
#ifdef PIXEL_USE_OFFSET
			FillPixels(0, LedCount, 0x000000);
#else
			fill_solid(LedFirst, LedCount, 0x000000);
#endif
		}
	}

	bool Update()
	{
		if (fxFadeOut > 0)
		{
			if ((millis() - lastUpdate) > (FADEOUT_DURATION / FADEOUT_STEPS))
			{
				DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: Fading out, " + String(fxFadeOut) + " steps remaing.");
#ifdef PIXEL_USE_OFFSET
				fadeToBlackBy(&leds[LedFirstNr], LedCount, (1024 / FADEOUT_STEPS));
#else
				fadeToBlackBy(LedFirst, LedCount, (1024 / FADEOUT_STEPS));
#endif
				fxFadeOut--;
				if (fxFadeOut == 0)
				{
#ifdef PIXEL_USE_OFFSET
					FillPixels(0, LedCount, 0x000000);
#else
					fill_solid(LedFirst, LedCount, 0x000000);
#endif
					if (crossFadeColors)
					{
						colorPalette = GenerateRGBPalette({0x000000, 0x000000});
					}
				}
				lastUpdate = millis();
				return true; // LEDs updated
			}
			return false; // LEDs not updated
		}

		if (!Active)
		{
			// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: SKIPPED update, not active.");
			return false; // LEDs not updated
		}

		int updateInterval = (1000 / (fxFps * fxFpsFactor));
		int sinceLastUpdate = (millis() - lastUpdate);
		if (sinceLastUpdate > updateInterval)
		{
			// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: " + String(sinceLastUpdate) + "ms since last update, target interval: " + String(updateInterval) + "ms.");
			// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: Updating group.");
			if (crossFadeColors)
			{
				// Cross-fade to new palette
				nblendPaletteTowardPalette(colorPalette, colorPaletteNew, CROSSFADE_MAX_CHANGE_PER_STEP);
			}
			if (effectFunc != NULL)
			{
				(this->*effectFunc)();

				if (fxAmountGlitter > 0)
				{
					FxGlitter(fxAmountGlitter);
				}
			}
			else
			{
				// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: ERROR, no FX function set.");
				lastUpdate = millis();
				return false;
			}
			NextFxStep();
			int timesToUpdate = constrain(sinceLastUpdate / updateInterval, 1, 8); // skip max 8 steps
			if (timesToUpdate > 1)
			{
				DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].Update: skipping " + String(timesToUpdate - 1) + " steps to catch up target FPS " + String(fxFps * fxFpsFactor) + ".");
				if (Active)
					NextFxStep(timesToUpdate - 1);
			}
			lastUpdate = millis();
			return true; // LEDs updated
		}

		return false; // LEDs not updated
	}

	void NextFxStep(int speed = 0)
	{
		if (fxDirection == FORWARD)
		{
			fxStep += speed > 0 ? speed : fxSpeed;
		}
		else // fxDirection == REVERSE
		{
			fxStep -= speed > 0 ? speed : fxSpeed;
		}

		if (onlyOnce)
		{
			// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].NextFxStep: " +
			// 			  "direction " + fxDirection +
			// 			  ", current FX step " + String(fxStep) +
			// 			  ", speed " + String(speed > 0 ? speed : fxSpeed) + ".");
			if ((fxDirection == FORWARD && fxStep >= 255) ||
				(fxDirection == REVERSE && fxStep <= 0))
			{
				DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].NextFxStep: Run only once, stopping at " + String(fxStep) + ".");
				onlyOnce = false;
				Stop();
			}
		}

		while (fxStep > 255)
		{
			fxStep -= 256;
		}
		while (fxStep < 0)
		{
			fxStep += 256;
		}

		// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].NextFxStep: next step >> " + String(fxStep) + ".");
	}

	void ReverseFxDirection()
	{
		if (fxDirection == FORWARD)
		{
			fxDirection = REVERSE;
			fxStep = 255;
		}
		else
		{
			fxDirection = FORWARD;
			fxStep = 0;
		}
	}
#pragma endregion

#pragma region SetPixel functions
	void FillPixels(int pos, int count, CRGB newcolor, mirror mirror = MIRROR0, bool blend = false)
	{
		for (int p = pos; p < (pos + count); p++)
			SetPixel(p, newcolor, mirror, blend);
	}

	void SetPixel(int pos, CRGB newcolor, mirror mirror = MIRROR0, bool blend = false)
	{
		int ledNormalNr = -1;
		int ledMirrorNr = -1;

		if (mirror == MIRROR1) // set even/odd as mirror values
		{
			int newPos = pos / 2;
			ledNormalNr = ((pos % 2) != 0)
							  ? CapLedPosition(newPos + LedOffset, LedCount)
							  : CapLedPosition((LedCount - newPos - 1) + LedOffset, LedCount);
		}
		else if (mirror == MIRROR2) // mirror each second value
		{
			int newPos = pos / 2;
			int mirrorPos = LedCount - newPos - 1;
			ledNormalNr = CapLedPosition(newPos + LedOffset, LedCount);
			ledMirrorNr = CapLedPosition(mirrorPos + LedOffset, LedCount);
		}
		else
		{
			ledNormalNr = pos + LedOffset;
		}

#ifdef PIXEL_USE_OFFSET
		CRGB *LedTargetNormal =
			(ledNormalNr >= 0) ? &leds[CapLedPosition(LedFirstNr + ledNormalNr + PIXEL_OFFSET, PIXEL_COUNT)] : nullptr;
		CRGB *LedTargetMirror =
			(ledMirrorNr >= 0) ? &leds[CapLedPosition(LedFirstNr + ledMirrorNr + PIXEL_OFFSET, PIXEL_COUNT)] : nullptr;
#else
		CRGB *LedTargetNormal =
			(ledNormalNr >= 0) ? &LedFirst[ledNormalNr] : nullptr;
		CRGB *LedTargetMirror =
			(ledMirrorNr >= 0) ? &LedFirst[ledMirrorNr] : nullptr;
#endif

		if (blend)
		{
			if (ledNormalNr >= 0)
				nblend(LedTargetNormal[0], newcolor, 128);
			if (ledMirrorNr >= 0)
				nblend(LedTargetMirror[0], newcolor, 128);
		}
		else
		{
			if (ledNormalNr >= 0)
				LedTargetNormal[0] = newcolor;
			if (ledMirrorNr >= 0)
				LedTargetMirror[0] = newcolor;
		}
	}

	int CapLedPosition(int ledPos, int ledCount)
	{
		int newPos = ledPos;
		while (newPos >= ledCount)
			newPos -= ledCount;
		while (newPos < 0)
			newPos += ledCount;
		return newPos;
	}

	uint8_t GetEasedFxStep(uint8_t step)
	{
		int newStep = step;
		switch (fxWave)
		{
		case SINUS:
			newStep = quadwave8(step);
			break;
			/*
		case EASEIN:
			//newStep = ease8InOutQuad(step / 2 + step % 2) * 2 + step % 2;
			newStep = (ease8InOutQuad(step >> 1) << 1) | (step & 0x01);
			break;
		case EASEOUT:
			//newStep = (ease8InOutQuad(128 + (step / 2 + step % 2)) - 128) * 2 + step % 2;
			newStep = (constrain(ease8InOutQuad(0x80 | (step >> 1)), 0, 0x7f)) | (step & 0x01);
			break;
		case EASEINOUT:
*/
		case EASEINOUT:
			newStep = ease8InOutQuad(step);
			break;
		default:
			newStep = fxStep;
			break;
		}
		return newStep;
	}
#pragma endregion

#pragma region Effects
	void FxStatic()
	{
		CRGB newColor = ColorFromPalette(colorPalette, 0);
#ifdef PIXEL_USE_OFFSET
		FillPixels(0, LedCount, newColor);
#else
		fill_solid(LedFirst, LedCount, newColor);
#endif
		for (int i = 0; i < LedCount; i++)
		{
			SetPixel(i, newColor);
		}
		//Stop();
	}

	void FxFade()
	{
		CRGB newColor = ColorFromPalette(colorPalette, fxStep);
		//fill_solid(LedFirst, LedCount, newColor);
		FillPixels(0, LedCount, newColor);

		NextFxStep();
	}

	void FxFill()
	{
#ifdef PIXEL_USE_OFFSET
		// fadeToBlackBy(&leds[LedFirstNr], LedCount, 16 * fxSpeed);
#else
		// fadeToBlackBy(LedFirst, LedCount, 16 * fxSpeed);
#endif
		int fxStep2 = (fxDirection == REVERSE) ? 255 - fxStep : fxStep;
		// uint8_t bri = map8(GetEasedFxStep(fxStep2), 32, 255);
		uint8_t bri = map8(scale8(fxStep2, fxStep2), 32, 255);
		//CRGB newColor = ColorFromPalette(colorPalette, fxStep2, bri);
		CRGB newColor = ColorFromPalette(colorPalette, fxStep2, bri);
		//int variant = LedCount < 64 ? (LedCount / 4) : (LedCount / 8);

		uint8_t pos = GetEasedFxStep(fxStep2); // + random(0 - variant, 0 + variant);
		int ledAmount = scale8(LedCount, pos);
		int ledPos = (fxDirection == REVERSE) ? LedCount - ledAmount : 0;
		if (ledAmount > 0)
			FillPixels(ledPos, ledAmount, newColor, fxMirror, false);

		NextFxStep();
	}

	void FxWave()
	{
		uint8_t waveLength = fxLength;
		uint8_t deltaWave = waveLength >= LedCount ? waveLength / LedCount : 1;
		//fill_rainbow(LedFirst, LedCount, fxStep, deltaHue);

		CRGB rgb;
		uint8_t wavePos = fxStep;
		for (int i = 0; i < LedCount; i++)
		{
			rgb = ColorFromPalette(colorPalette, wavePos);
			SetPixel((LedCount - i - 1), rgb, fxMirror, false); // don't blend
			wavePos += deltaWave;
		}

		NextFxStep();
	}

	void FxRainbow()
	{
		uint8_t rainBowLength = fxLength;
		uint8_t deltaHue = rainBowLength >= LedCount ? rainBowLength / LedCount : 1;
		//fill_rainbow(LedFirst, LedCount, fxStep, deltaHue);

		CHSV hsv;
		hsv.hue = fxStep;
		hsv.sat = 240;
		hsv.val = 255;
		for (int i = 0; i < LedCount; i++)
		{
			//LedFirst[i] = hsv;
			SetPixel((LedCount - i - 1), hsv, fxMirror, false); // don't blend
			hsv.hue += deltaHue;
		}

		NextFxStep();
	}

	void FxGlitter(uint8_t chanceOfGlitter = 80)
	{
		if (random8() < chanceOfGlitter)
		{
#ifdef PIXEL_USE_OFFSET
			SetPixel(random16(LedCount), CRGB::White, MIRROR0, true);
#else
			LedFirst[random16(LedCount)] += CRGB::White;
#endif
		}
	}

	void FxConfetti()
	{
#ifdef PIXEL_USE_OFFSET
		// fadeToBlackBy(&leds[LedFirstNr], LedCount, 16 * fxSpeed);
		fadeToBlackBy(&leds[LedFirstNr], LedCount, 8 * fxSpeed);
#else
		// fadeToBlackBy(LedFirst, LedCount, 16 * fxSpeed);
		fadeToBlackBy(LedFirst, LedCount, 8 * fxSpeed);
#endif
		int pos = random16(LedCount);
		SetPixel(pos, ColorFromPalette(colorPalette, fxStep + random8(64)), MIRROR0, false); //true);
		NextFxStep();
	}

	void FxComet()
	{
#ifdef PIXEL_USE_OFFSET
		// fadeToBlackBy(&leds[LedFirstNr], LedCount, 16 * fxSpeed);
		fadeToBlackBy(&leds[LedFirstNr], LedCount, 8 * fxSpeed);
#else
		// fadeToBlackBy(LedFirst, LedCount, 16 * fxSpeed);
		fadeToBlackBy(LedFirst, LedCount, 8 * fxSpeed);
#endif
		bool isReverse = fxDirection == direction::REVERSE;
		int fxStep2 = isReverse ? 255 - fxStep : fxStep;
		//int variant = LedCount < 64 ? (LedCount / 4) : (LedCount / 8);
		int variant = LedCount / 8;
		int pos = GetEasedFxStep(fxStep2);
		pos = constrain(pos + random(0 - variant, 0 + variant), 0, 255);
		//pos = lerp8by8(0, LedCount, pos);
		pos = map8(pos, 0, LedCount);
		pos = constrain(pos, 0, LedCount);
		if (isReverse)
			pos = LedCount - pos;
		int bright = random(64, 255);
		// uint8_t colpos = (fxStep2 * 1.5) + random8(16);
		uint8_t colpos = fxStep2 + random8(16);
		// DEBUG_GRP_PRINTLN("GRP[" + String(GroupID) + "].FxComet: " +
		// 			  "step " + String(fxStep) +
		// 			  ", pixel " + String(pos) +
		// 			  ", color " + String(colpos) + ".");
		SetPixel(pos, ColorFromPalette(colorPalette, colpos, bright), fxMirror, true);														// blending OK
		SetPixel(constrain(pos + random(1, variant > 1), 0, LedCount), ColorFromPalette(colorPalette, colpos, bright > 1), fxMirror, true); // blending OK
		SetPixel(constrain(pos - random(1, variant > 1), 0, LedCount), ColorFromPalette(colorPalette, colpos, bright > 1), fxMirror, true); // blending OK
		NextFxStep();
	}

	void FxFire()
	{
		// Array of temperature readings at each simulation cell
		byte heat[LedCount];

		// Step 1.  Cool down every cell a little
		for (int i = 0; i < LedCount; i++)
		{
			heat[i] = qsub8(heat[i], random8(0, ((FIRE_COOLING * 10) / LedCount) + 2));
		}

		// Step 2.  Heat from each cell drifts 'up' and diffuses a little
		for (int k = LedCount - 1; k >= 2; k--)
		{
			heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
		}

		// Step 3.  Randomly ignite new 'sparks' of heat near the bottom
		if (random8() < FIRE_SPARKING)
		{
			int y = random8(LedCount / 8);
			heat[y] = qadd8(heat[y], random8(160, 255));
		}

		// Step 4.  Map from heat cells to LED colors
		for (int j = 0; j < LedCount; j++)
		{
			// Scale the heat value from 0-255 down to 0-240
			// for best results with color palettes.
			byte colorindex = scale8(heat[j], 240);
			CRGB color = ColorFromPalette(colorPalette, colorindex);
			SetPixel(j, color, fxMirror);
		}
		NextFxStep();
	}

	void FxColorWaves()
	{
		static uint16_t sPseudotime = 0;
		static uint16_t sLastMillis = 0;
		static uint16_t sHue16 = 0;

		// uint8_t sat8 = beatsin88(87, 220, 250);
#ifdef FxColorWavesWithBrightness
		uint8_t brightdepth = beatsin88(341, 96, 224);
		uint16_t brightnessthetainc16 = beatsin88(203, (25 * 256), (40 * 256));
#endif
		uint8_t msmultiplier = beatsin88(147, 23, 60);

		uint16_t hue16 = sHue16;
		// uint16_t hueinc16 = beatsin88(113, 300, 1500);
		uint16_t hueinc16 = beatsin88(113, 150, 600);

		uint16_t ms = millis();
		uint16_t deltams = ms - sLastMillis;
		sLastMillis = ms;
		sPseudotime += deltams * msmultiplier;
		sHue16 += deltams * beatsin88(400, 5, 9);
#ifdef FxColorWavesWithBrightness
		uint16_t brightnesstheta16 = sPseudotime;
#endif

		for (uint16_t i = 0; i < LedCount; i++)
		{
			hue16 += hueinc16;
			uint8_t hue8 = hue16 / 256;
			/*
			uint16_t h16_128 = hue16 >> 7;
			if (h16_128 & 0x100)
			{
				hue8 = 255 - (h16_128 >> 1);
			}
			else
			{
				hue8 = h16_128 >> 1;
			}
			*/

#ifdef FxColorWavesWithBrightness
			brightnesstheta16 += brightnessthetainc16;
			uint16_t b16 = sin16(brightnesstheta16) + 32768;
			uint16_t bri16 = (uint32_t)((uint32_t)b16 * (uint32_t)b16) / 65536;
			uint8_t bri8 = (uint32_t)(((uint32_t)bri16) * brightdepth) / 65536;
			bri8 += (255 - brightdepth);
#endif
			uint16_t pixelnumber = i;
			/*
			if (i < 8)
			{
				DEBUG_GRP_PRINT("GRP[" + String(GroupID) + "].FxColorWaves: Getting palette color for pixel ");
				DEBUG_GRP_PRINT(pixelnumber);
				DEBUG_GRP_PRINT(" based on hue8 ");
				DEBUG_GRP_PRINT(hue8, HEX);
				DEBUG_GRP_PRINT(" hue16 ");
				DEBUG_GRP_PRINT(hue16, HEX);
				DEBUG_GRP_PRINT(" at index ");
				DEBUG_GRP_PRINT(index, HEX);
#ifdef FxColorWavesWithBrightness
				DEBUG_GRP_PRINT(" with bri8 ");
				DEBUG_GRP_PRINT(bri8, HEX);
#endif
				DEBUG_GRP_PRINTLN(".");
			}
			*/
#ifdef FxColorWavesWithBrightness
			// CRGB newcolor = ColorFromPalette(colorPalette, hue8, bri8);
			CRGB newcolor = ColorFromPalette16(colorPalette, hue16, bri8);
#else
			// CRGB newcolor = ColorFromPalette(colorPalette, hue8);
			CRGB newcolor = ColorFromPalette16(colorPalette, hue16);
#endif
			// SetPixel(pixelnumber, newcolor, fxMirror, false); // don't blend
			SetPixel(pixelnumber, newcolor, fxMirror, true); // blend?
		}
	}

	void FxNoise()
	{
		static uint8_t lastMillis = 0;
		static uint8_t slowness = 2;
		static uint32_t noiseYScale = 0;

		static uint16_t indexMin = 0x7fff;
		static uint16_t indexMax = 0x8000;
		static uint16_t indexRange = indexMax - indexMin;

		uint8_t currentMillis = millis();
		if (uint8_t(currentMillis - lastMillis) > 8)
		{
			lastMillis = currentMillis;
			//noiseYScale++;
			noiseYScale += uint16_t(63) << slowness;
		}

		uint16_t indexMinNew = indexMin;
		uint16_t indexMaxNew = indexMax;
		for (uint16_t i = 0; i < LedCount; i++)
		{
			// X location is constant, but we move along the Y at the rate of millis()
			//uint8_t index = inoise8(uint16_t(i) * 20, noiseYScale);
			uint16_t index = inoise16(uint32_t((i * 128) / LedCount) << 12, noiseYScale);
			/*
			uint8_t hi4 = (index >> 8) & 0xF0; // take the highest 4 bits
			uint8_t lo4 = (index >> 4) & 0x0F; // take the next-to-highest 8 bits, ignore the lowest 4 bits
			uint8_t index8 = hi4 | lo4;
			*/

			// Dynamically scale calculated index range to get values between 0x0000 and 0xffff
			if (indexMin > index)
			{
				if (indexMinNew > index)
					indexMinNew = index;
				index = indexMin;
			}
			if (indexMax < index)
			{
				if (indexMaxNew < index)
					indexMaxNew = index;
				index = indexMax;
			}
			uint16_t newIndex = ((uint32_t)(index - indexMin) * uint32_t(0xffff)) / indexRange;
			uint8_t index8 = newIndex >> 8;

			uint16_t pixelnumber = i;
			/*
			if (i < 8)
			{
				DEBUG_GRP_PRINT("GRP[" + String(GroupID) + "].FxNoise: Getting palette color for pixel ");
				DEBUG_GRP_PRINT(pixelnumber);
				// DEBUG_GRP_PRINT(" for noiseYScale ");
				// DEBUG_GRP_PRINT(noiseYScale);
				DEBUG_GRP_PRINT(" at index ");
				DEBUG_GRP_PRINT(newIndex, HEX);
				DEBUG_GRP_PRINT(" based on index16 ");
				DEBUG_GRP_PRINT(index, HEX);
				// DEBUG_GRP_PRINT(" as index8 ");
				// DEBUG_GRP_PRINT(index8, HEX);
				DEBUG_GRP_PRINTLN(".");
			}
			*/
			// CRGB newcolor = ColorFromPalette(colorPalette, index8);
			CRGB newcolor = ColorFromPalette16(colorPalette, newIndex);
			SetPixel(pixelnumber, newcolor, fxMirror, false); // don't blend
		}

		// Dynamically scale calculated index range to get values between 0x0000 and 0xffff
		if (indexMin > indexMinNew)
		{
			indexMin = indexMinNew;
			indexRange = indexMax - indexMin;
			DEBUG_GRP_PRINT("GRP[" + String(GroupID) + "].FxNoise: new indexMin=");
			DEBUG_GRP_PRINT(indexMin, HEX);
			DEBUG_GRP_PRINT(", new indexRange=");
			DEBUG_GRP_PRINTLN(indexRange, HEX);
		}
		if (indexMax < indexMaxNew)
		{
			indexMax = indexMaxNew;
			indexRange = indexMax - indexMin;
			DEBUG_GRP_PRINT("GRP[" + String(GroupID) + "].FxNoise: new indexMax=");
			DEBUG_GRP_PRINT(indexMax, HEX);
			DEBUG_GRP_PRINT(", new indexRange=");
			DEBUG_GRP_PRINTLN(indexRange, HEX);
		}
	}
#pragma endregion

#pragma region Static ColorPalette functions
	static CRGBPalette16 GenerateRGBPalette(std::vector<CRGB> colors)
	{
		CRGB nc[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		int colCount = colors.size();
		for (int c = 0; c < colCount; c++)
		{
			uint16_t trgtIdx1 = ((c * 16) / colCount);
			uint16_t trgtIdx2 = (((c + 1) * 16) / colCount) - 1;
			if (trgtIdx2 > 15)
				trgtIdx2 = 15;
			if (trgtIdx2 < 0)
				trgtIdx2 = 0;
			CRGB col1 = colors[c];
			CRGB col2 = (c + 1 < colCount) ? colors[c + 1] : colors[0];
			fill_gradient_RGB(&(nc[0]), trgtIdx1, col1, trgtIdx2, col2);
		}
		return CRGBPalette16(
			nc[0], nc[1], nc[2], nc[3], nc[4], nc[5], nc[6], nc[7],
			nc[8], nc[9], nc[10], nc[11], nc[12], nc[13], nc[14], nc[15]);
	}

	// Copied from:
	// https://github.com/FastLED/FastLED/blob/master/colorutils.cpp
	// Modified to have a 16-bit index, for more precise colors (though only
	// 12 bits are actually used).
	static CRGB ColorFromPalette16(const CRGBPalette16 &pal, uint16_t index, uint8_t brightness = 0xff, TBlendType blendType = LINEARBLEND)
	{
		uint8_t hi4 = index >> 12; // take the highest 4 bits
		uint8_t lo8 = index >> 4;  // take the next-to-highest 8 bits, ignore the lowest 4 bits

		// const CRGB* entry = &(pal[0]) + hi4;
		// since hi4 is always 0..15, hi4 * sizeof(CRGB) can be a single-byte value,
		// instead of the two byte 'int' that avr-gcc defaults to.
		// So, we multiply hi4 X sizeof(CRGB), giving hi4XsizeofCRGB;
		uint8_t hi4XsizeofCRGB = hi4 * sizeof(CRGB);
		// We then add that to a base array pointer.
		const CRGB *entry = (CRGB *)((uint8_t *)(&(pal[0])) + hi4XsizeofCRGB);

		uint8_t blend = lo8 && (blendType != NOBLEND);

		uint8_t red1 = entry->red;
		uint8_t green1 = entry->green;
		uint8_t blue1 = entry->blue;

		if (blend)
		{

			if (hi4 == 15)
			{
				entry = &(pal[0]);
			}
			else
			{
				entry++;
			}

			uint8_t f2 = lo8;
			uint8_t f1 = 255 - f2;

			//    rgb1.nscale8(f1);
			uint8_t red2 = entry->red;
			red1 = scale8_LEAVING_R1_DIRTY(red1, f1);
			red2 = scale8_LEAVING_R1_DIRTY(red2, f2);
			red1 += red2;

			uint8_t green2 = entry->green;
			green1 = scale8_LEAVING_R1_DIRTY(green1, f1);
			green2 = scale8_LEAVING_R1_DIRTY(green2, f2);
			green1 += green2;

			uint8_t blue2 = entry->blue;
			blue1 = scale8_LEAVING_R1_DIRTY(blue1, f1);
			blue2 = scale8_LEAVING_R1_DIRTY(blue2, f2);
			blue1 += blue2;

			cleanup_R1();
		}

		if (brightness != 255)
		{
			if (brightness)
			{
				brightness++; // adjust for rounding
				// Now, since brightness is nonzero, we don't need the full scale8_video logic;
				// we can just to scale8 and then add one (unless scale8 fixed) to all nonzero inputs.
				if (red1)
				{
					red1 = scale8_LEAVING_R1_DIRTY(red1, brightness);
#if !(FASTLED_SCALE8_FIXED == 1)
					red1++;
#endif
				}
				if (green1)
				{
					green1 = scale8_LEAVING_R1_DIRTY(green1, brightness);
#if !(FASTLED_SCALE8_FIXED == 1)
					green1++;
#endif
				}
				if (blue1)
				{
					blue1 = scale8_LEAVING_R1_DIRTY(blue1, brightness);
#if !(FASTLED_SCALE8_FIXED == 1)
					blue1++;
#endif
				}
				cleanup_R1();
			}
			else
			{
				red1 = 0;
				green1 = 0;
				blue1 = 0;
			}
		}

		return CRGB(red1, green1, blue1);
	}
#pragma endregion
};
