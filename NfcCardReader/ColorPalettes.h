#include <Arduino.h>
#include "SerialDebug.h"
#include "FastLedInclude.h"
// #include <ArduinoSTL.h>
#include <vector>
#include <map>

// #define DUMP_PALETTE

// #define BOOST_COLORS
#define BOOST_SAT_MIN 160
#define BOOST_SAT_INCR 64
#define BOOST_VAL_MIN 128
#define BOOST_VAL_INCR 64

// Helper macro
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

/*const*/ std::vector<String> ColorNames;
/*const*/ std::map<String, std::vector<CRGB>> ColorPalettes;

const CRGB LED_BLACK = 0x000000;
const CRGB LED_GRAY_DARK = 0x101010;
const CRGB LED_GRAY_MEDIUM = 0x202020;
const CRGB LED_GRAY_LIGHT = 0x404040;
const CRGB LED_WHITE = 0xFFFFFF;

#pragma region "DEBUG"
void PrintHex8(uint8_t data) // prints 8-bit data in hex with leading zeroes
{
    if (data < 0x10)
        Serial.print("0");
    DEBUG_PRINT(data, HEX);
}

void DumpPalette(std::vector<CRGB> pal)
{
    DEBUG_PRINT("Colors: ");
    CRGB rgbcolor;
    for (int c = 0; c < pal.size(); c++)
    {
        rgbcolor = pal.at(c);
        DEBUG_PRINT("0x");
        PrintHex8(rgbcolor.r);
        PrintHex8(rgbcolor.g);
        PrintHex8(rgbcolor.b);
        if (c != pal.size() - 1)
        {
            DEBUG_PRINT(",");
        }
    }
    DEBUG_PRINTLN(".");
}
#pragma endregion

#pragma region "WorldCup 2018"
const std::vector<String> WorldCupTeamKeys = {
    "RUS", "KSA", "EGY", "URU", // Group A
    "POR", "ESP", "MAR", "IRN", // Group B
    "FRA", "AUS", "PER", "DEN", // Group C
    "ARG", "ISL", "CRO", "NGA", // Group D
    "BRA", "SUI", "CRC", "SRB", // Group E
    "GER", "MEX", "SWE", "KOR", // Group F
    "BEL", "PAN", "TUN", "ENG", // Group G
    "POL", "SEN", "COL", "JPN", // Group H
};

const std::map<String, String> WorldCupTeamNames =
    {
        {"RUS", "Russland"},
        {"KSA", "S.Arabien"},
        {"EGY", "Ägypten"},
        {"URU", "Uruguay"},
        {"POR", "Portugal"},
        {"ESP", "Spanien"},
        {"MAR", "Marokko"},
        {"IRN", "Iran"},
        {"FRA", "Frankreich"},
        {"AUS", "Australien"},
        {"PER", "Peru"},
        {"DEN", "Dänemark"},
        {"ARG", "Argentinien"},
        {"ISL", "Island"},
        {"CRO", "Kroatien"},
        {"NGA", "Nigeria"},
        {"BRA", "Brasilien"},
        {"SUI", "Schweiz"},
        {"CRC", "Costa Rica"},
        {"SRB", "Serbien"},
        {"GER", "Deutschland"},
        {"MEX", "Mexiko"},
        {"SWE", "Schweden"},
        {"KOR", "Korea"},
        {"BEL", "Belgien"},
        {"PAN", "Panama"},
        {"TUN", "Tunesien"},
        {"ENG", "England"},
        {"POL", "Polen"},
        {"SEN", "Senegal"},
        {"COL", "Colombia"},
        {"JPN", "Japan"},
};

const std::map<String, std::vector<CRGB>> WorldCupTeamColors =
    {
        // {"TEST1", {0xFFFFFF, 0x808080, 0x404040}},
        // {"TEST2", {0x404040, 0x202020, 0x101010}},
        // {"RUS", {0xFFFFFF, 0xFFFFFF, 0x0039A6, 0x0039A6, 0xD52B1E, 0xD52B1E}},
        {"RUS", {0xFFFFFF, 0x0039A6, 0xD52B1E}},
        {"KSA", {0x006C35, 0xFFFFFF, 0x006C35}},
        {"EGY", {0xCE1126, 0xFFFFFF, 0x080808}},
        {"URU", {0x0038A8, 0xFFFFFF, 0xFCD116, 0x0038A8, 0xFFFFFF, 0x0038A8}},
        {"POR", {0x006600, 0xFF0000, 0xFF0000}},
        {"ESP", {0xC60B1E, 0xFFC400, 0xC60B1E}},
        {"MAR", {0xC1272D, 0x006233, 0xC1272D}},
        {"IRN", {0x239F40, 0xFFFFFF, 0xDA0000}},
        {"FRA", {0x002395, 0xFFFFFF, 0xED2939}},
        {"AUS", {0x012169, 0x012169, 0xFFFFFF, 0xE4002B, 0xFFFFFF, 0x012169}},
        {"PER", {0xD91023, 0xFFFFFF, 0xD91023}},
        {"DEN", {0xC60C30, 0xFFFFFF, 0xC60C30, 0xFFFFFF, 0xC60C30, 0xC60C30}},
        {"ARG", {0x74ACDF, 0xFFFFFF, 0xF6B40E}},
        {"ISL", {0x02529C, 0xFFFFFF, 0xDC1E35, 0xFFFFFF, 0x02529C, 0x02529C}},
        {"CRO", {0xFF0000, 0xFFFFFF, 0x171796}},
        {"NGA", {0x008751, 0xFFFFFF, 0x008751}},
        {"BRA", {0x009B3A, 0xFEDF00, 0x002776, 0x002776, 0xFEDF00, 0x009B3A}},
        {"SUI", {0xD52B1E, 0xFFFFFF, 0xD52B1E}},
        {"CRC", {0x002B7F, 0xFFFFFF, 0xCE1126, 0xCE1126, 0xFFFFFF, 0x002B7F}},
        {"SRB", {0xC6363C, 0x0C4076, 0xFFFFFF}},
        {"GER", {0x080808, 0xDD0000, 0xFFCE00}},
        {"MEX", {0x006847, 0xFFFFFF, 0xCE1126}},
        {"SWE", {0x006BA8, 0xFECD01, 0x006BA8}},
        {"KOR", {0xFFFFFF, 0xC60C30, 0xC60C30, 0x003478, 0x003478, 0xFFFFFF}},
        {"BEL", {0x080808, 0xFAE042, 0xED2939}},
        {"PAN", {0xFFFFFF, 0xD21034, 0xD21034, 0x005293, 0x005293, 0xFFFFFF}},
        {"TUN", {0xE70013, 0xFFFFFF, 0xE70013}},
        {"ENG", {0xFFFFFF, 0xCF081F, 0xFFFFFF}},
        {"POL", {0xDC143C, 0xDC143C, 0xFFFFFF, 0xFFFFFF}},
        // {"POL", {0xDC143C, 0xFFFFFF}},
        {"SEN", {0x00853F, 0xFDEF42, 0xE31B23}},
        {"COL", {0xFCD116, 0x003893, 0xCE1126}},
        {"JPN", {0xFFFFFF, 0xBC002D, 0xFFFFFF}},
};
#pragma endregion

#pragma region "Palette Creation"
CRGB GetHSV(CRGB rgbcolor)
{
    return rgb2hsv_approximate(rgbcolor);
}

CRGB BoostColor(CRGB origrgbcolor)
{
    CRGB rgbcolor = origrgbcolor;
#ifdef BOOST_COLORS
    CHSV hsvcolor = rgb2hsv_approximate(rgbcolor);
    if (hsvcolor.s < BOOST_SAT_MIN || hsvcolor.v < BOOST_VAL_MIN)
    {
        if (rgbcolor.r == rgbcolor.g && rgbcolor.g == rgbcolor.b)
        {
            DEBUG_PRINT("Boost: Keeping 0x");
            PrintHex8(rgbcolor.r);
            PrintHex8(rgbcolor.g);
            PrintHex8(rgbcolor.b);
            DEBUG_PRINTLN(", pure grey/white.");
        }
        else
        {
            DEBUG_PRINT("Boost: Boosting 0x");
            PrintHex8(rgbcolor.r);
            PrintHex8(rgbcolor.g);
            PrintHex8(rgbcolor.b);
            DEBUG_PRINT(",");
            DEBUG_PRINT(" S:");
            DEBUG_PRINT(hsvcolor.s);
            if (hsvcolor.s < BOOST_SAT_MIN)
            {
                DEBUG_PRINT("->");
                while (hsvcolor.s < BOOST_SAT_MIN)
                    hsvcolor.s += BOOST_SAT_INCR;
                DEBUG_PRINT(hsvcolor.s);
            }
            DEBUG_PRINT(", V:");
            DEBUG_PRINT(hsvcolor.v);
            if (hsvcolor.v < BOOST_VAL_MIN)
            {
                DEBUG_PRINT("->");
                while (hsvcolor.v < BOOST_VAL_MIN)
                    hsvcolor.v += BOOST_VAL_INCR;
                DEBUG_PRINT(hsvcolor.v);
            }
            rgbcolor = CRGB(hsvcolor);
            DEBUG_PRINT(", now 0x");
            PrintHex8(rgbcolor.r);
            PrintHex8(rgbcolor.g);
            PrintHex8(rgbcolor.b);
            DEBUG_PRINTLN(".");
        }
    }
    else
    {
        DEBUG_PRINT("Boost: Keeping 0x");
        PrintHex8(rgbcolor.r);
        PrintHex8(rgbcolor.g);
        PrintHex8(rgbcolor.b);
        uint8_t minSat = BOOST_SAT_MIN;
        uint8_t minVal = BOOST_VAL_MIN;
        DEBUG_PRINTLN(", S:" + String(hsvcolor.s) + ">" + String(minSat) + " and V:" + String(hsvcolor.v) + ">" + String(minVal) + ".");
    }
#endif
    return rgbcolor;
}

void AddColorPalette(
    String palName,
    std::vector<CRGB> palColors,
    bool boostSaturation = false)
{
    DEBUG_PRINT("ColorPalettes: adding color palette '");
    DEBUG_PRINT(palName);
    DEBUG_PRINTLN("'.");
    ColorNames.push_back(palName);
    if (!boostSaturation || palColors == (std::vector<CRGB>)NULL)
    {
        ColorPalettes[palName] = palColors;
    }
    else
    {
        std::vector<CRGB> palColorsBoosted;
        CRGB rgbcolor;
        int colCount = palColors.size();
        for (int c = 0; c < colCount; c++)
        {
            rgbcolor = BoostColor(palColors[c]);
            palColorsBoosted.push_back(rgbcolor);
        }
        ColorPalettes[palName] = palColorsBoosted;
    }
#ifdef DUMP_PALETTE
    DumpPalette(ColorPalettes[palName]);
#endif
}
#pragma endregion

#pragma region "Palette Generators"
std::vector<CRGB> SimplePaletteFromColor(
    CRGB baseCol, int variants = 1, uint8_t variantBy = 16)
{
    std::vector<CRGB> newPal;
    CRGB rgb = CRGB(baseCol);
    newPal.push_back(rgb);
    for (int i = 0; i < variants; i++)
    {
        CHSV hsv = rgb2hsv_approximate(rgb);
        hsv.h += variantBy;
        rgb = CRGB(hsv);
        newPal.push_back(rgb);
    }
    return newPal;
}

std::vector<CRGB> AdjacentPaletteFromHue(
    uint8_t hue, uint8_t sat,
    bool addCompl = false, uint8_t distance = 30)
{
    uint8_t dist8 = (255 * distance) / 360;
    std::vector<CRGB> newPal;
    CHSV hsvBase = CHSV(hue, sat, 255);
    newPal.push_back(CRGB(hsvBase)); // base color
    CHSV hsvNew = CHSV(hue + dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // right adjacent
    if (addCompl)
    {
        hsvNew = CHSV(hue - 128, sat, 255);
        newPal.push_back(CRGB(hsvNew)); // complementary
    }
    else
    {
        newPal.push_back(CRGB(hsvBase)); // base color
    }
    hsvNew = CHSV(hue - dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // left adjacent
    return newPal;
}

std::vector<CRGB> TriadPaletteFromHue(
    uint8_t hue, uint8_t sat,
    bool addCompl = false, uint8_t distance = 30)
{
    return AdjacentPaletteFromHue(hue, distance + 90, addCompl);
}

std::vector<CRGB> AnalogousPaletteFromHue(
    uint8_t hue, uint8_t sat,
    bool addCompl = false, uint8_t distance = 15)
{
    uint8_t dist8 = (255 * distance) / 360;
    std::vector<CRGB> newPal;
    CHSV hsvBase = CHSV(hue, sat, 255);
    newPal.push_back(CRGB(hsvBase)); // base color
    CHSV hsvNew = CHSV(hue - dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // first left analogous
    hsvNew = CHSV(hue - 2 * dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // second left analogous
    if (addCompl)
    {
        hsvNew = CHSV(hue - 128, sat, 255);
        newPal.push_back(CRGB(hsvNew)); // complementary
    }
    else
    {
        newPal.push_back(CRGB(hsvBase)); // base color
    }
    hsvNew = CHSV(hue + 2 * dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // second right analogous
    hsvNew = CHSV(hue + dist8, sat, 255);
    newPal.push_back(CRGB(hsvNew)); // first right analogous
    if (addCompl)
    {
        // stronger base color
        newPal.push_back(CRGB(hsvBase)); // base color
    }
    return newPal;
}

std::vector<CRGB> GeneratePaletteFromHue(
    String method, uint8_t hue, uint8_t sat)
{
    if (method == "Farbe statisch")
        return {(CHSV(hue, sat, 255))};
    if (method == "Farbmix - Analog")
        return AnalogousPaletteFromHue(hue, sat);
    if (method == "Farbmix Analog+Komplement")
        return AnalogousPaletteFromHue(hue, sat, true);
    if (method == "Farbmix Angrenzend")
        return AdjacentPaletteFromHue(hue, sat);
    if (method == "Farbmix Angrenzend+Komplement")
        return AdjacentPaletteFromHue(hue, sat, true);
    if (method == "Farbmix Triade")
        return TriadPaletteFromHue(hue, sat);
    if (method == "Farbmix Triade+Komplement")
        return TriadPaletteFromHue(hue, sat, true);

    return SimplePaletteFromColor(CHSV(hue, sat, 255), 1, 32);
    // return {(CHSV(hue, sat, 255))};
}

// const std::map<String, uint8_t> HueValues =
//     {
//         {"Rot", HUE_RED},
//         {"Orange", HUE_ORANGE},
//         {"Gelb", HUE_YELLOW},
//         {"Grün", HUE_GREEN},
//         {"Aqua", HUE_AQUA},
//         {"Blau", HUE_BLUE},
//         {"Purpur", HUE_PURPLE},
//         {"Pink", HUE_PINK}};
// const std::vector<String> HuesNames = {
//     "Pink",
//     "Aqua",
//     "Orange",
//     "Purpur",
//     "Grün",
//     "Rot",
//     "Blau",
//     "Gelb"};
#pragma endregion

#pragma region "WorldCup 2018"
std::vector<CRGB> CreateTeamColorPalette(String teamKey)
{
    DEBUG_PRINTLN("Initializing team " + teamKey + "...");

    DEBUG_PRINTLN("Team:" + teamKey + " -> " + WorldCupTeamNames.find(teamKey)->second);

    std::vector<CRGB> teamColors = WorldCupTeamColors.find(teamKey)->second;
    std::vector<CRGB> teamPalette = {};
    // teamPalette.push_back(LED_BLACK);
    teamPalette.push_back(LED_GRAY_DARK);
    for (int tc = 0; tc < teamColors.size(); tc++)
    {
        teamPalette.push_back(BoostColor(teamColors.at(tc)));
    }
    // teamPalette.push_back(LED_GRAY_MEDIUM);
    // teamPalette.push_back(LED_GRAY_DARK);
    // teamPalette.push_back(LED_GRAY_MEDIUM);
    // for (int tc = 0; tc < teamColors.size(); tc++)
    // {
    //     teamPalette.push_back(BoostColor(teamColors.at(teamColors.size() - tc)));
    // }
    return teamPalette;
}
#pragma endregion

#pragma region "Palette Management"
void InitColorPalettes()
{
    DEBUG_PRINTLN("Initializing color palettes...");

    // for (int h = 0; h < HuesNames.size(); h++)
    // {
    //     String hueName = HuesNames[h];
    //     uint8_t hue = HueValues.at(hueName);
    //     /*
    //     //AddColorPalette(""/*"Adjacent_" + String(hue), AdjacentPaletteFromHue(hue, 255), false);
    //     //AddColorPalette(""/*"Adjacent_" + String(hue) + "_C", AdjacentPaletteFromHue(hue, 255, true), false);
    //     //AddColorPalette(""/*"Triad_" + String(hue), TriadPaletteFromHue(hue, 255), false);
    //     //AddColorPalette(""/*"Triad_" + String(hue) + "_C", TriadPaletteFromHue(hue, 255, true), false);
    //     AddColorPalette(""/*"Analogous_" + String(hue), AnalogousPaletteFromHue(hue, 255), false);
    //     AddColorPalette(""/*"Analogous" + String(hue) + "_C", AnalogousPaletteFromHue(hue, 255, true), false);
    //     */
    //     AddColorPalette(hueName, AnalogousPaletteFromHue(hue, 255), false);
    //     AddColorPalette(hueName + "+", AnalogousPaletteFromHue(hue, 255, true), false);
    // }

    // Placeholders for dynamic palettes from hue
    AddColorPalette("Farbe statisch", (std::vector<CRGB>)NULL /*AnalogousPaletteFromHue(hue, 255)*/, false);
    AddColorPalette("Farbmix - Analog", (std::vector<CRGB>)NULL /*AnalogousPaletteFromHue(hue, 255)*/, false);
    AddColorPalette("Farbmix Analog+Komplement", (std::vector<CRGB>)NULL /*AnalogousPaletteFromHue(hue, 255, true)*/, false);
    AddColorPalette("Farbmix Angrenzend", (std::vector<CRGB>)NULL /*AdjacentPaletteFromHue(hue, 255)*/, false);
    AddColorPalette("Farbmix Angrenzend+Komplement", (std::vector<CRGB>)NULL /*AdjacentPaletteFromHue(hue, 255, true)*/, false);
    AddColorPalette("Farbmix Triade", (std::vector<CRGB>)NULL /*TriadPaletteFromHue(hue, 255)*/, false);
    AddColorPalette("Farbmix Triade+Komplement", (std::vector<CRGB>)NULL /*TriadPaletteFromHue(hue, 255, true)*/, false);

    // World Cup 2018
    for (int teamNr = 0; teamNr < WorldCupTeamKeys.size(); teamNr++)
    {
        String teamKey = WorldCupTeamKeys[teamNr];
        AddColorPalette("WM18 " + teamKey, CreateTeamColorPalette(teamKey), false);
    }

    // Add some more pretty palettes :-)
    AddColorPalette("Regenbogen" /*"Rainbow"*/, {CRGB(0xFF0000), CRGB(0xD52A00), CRGB(0xAB5500), CRGB(0xAB7F00), CRGB(0xABAB00), CRGB(0x56D500), CRGB(0x00FF00), CRGB(0x00D52A), CRGB(0x00AB55), CRGB(0x0056AA), CRGB(0x0000FF), CRGB(0x2A00D5), CRGB(0x5500AB), CRGB(0x7F0081), CRGB(0xAB0055), CRGB(0xD5002B)});
    AddColorPalette("Party" /*"Party"*/, {CRGB(0x5500AB), CRGB(0x84007C), CRGB(0xB5004B), CRGB(0xE5001B), CRGB(0xE81700), CRGB(0xB84700), CRGB(0xAB7700), CRGB(0xABAB00), CRGB(0xAB5500), CRGB(0xDD2200), CRGB(0xF2000E), CRGB(0xC2003E), CRGB(0x8F0071), CRGB(0x5F00A1), CRGB(0x2F00D0), CRGB(0x0007F9)});
    AddColorPalette("Pop" /*"Pop"*/, {CRGB(0xAAFF00), CRGB(0xFFAA00), CRGB(0xFF00AA), CRGB(0xAA00FF), CRGB(0x00AAFF)});

    // Colourlovers Palettes
    AddColorPalette("Blues" /*"Blues"*/, {CRGB(0x4AD8DB), CRGB(0xD30081), CRGB(0x780072), CRGB(0x1A46F4), CRGB(0x02155B)});
    AddColorPalette("Einhorn" /*"Unicorn1"*/, {CRGB(0xEEF6D0), CRGB(0xBF7DB7), CRGB(0x40F9E8), CRGB(0x49C1BB), CRGB(0x85038C)}); // Exclude?
    AddColorPalette("Tag und Nacht 1" /*"DayAndNight"*/, {CRGB(0x00C17D), CRGB(0x00E999), CRGB(0xEF9000), CRGB(0xEDEA01), CRGB(0xFFF439)});
    AddColorPalette("Tag und Nacht 2" /*"NightAndDay1"*/, {CRGB(0x029DAF), CRGB(0xE5D599), CRGB(0xFFC219), CRGB(0xF07C19), CRGB(0xE32551)});
    AddColorPalette("Frostig" /*"Frosty"*/, {CRGB(0x645067), CRGB(0x0095BD), CRGB(0x3CCB91), CRGB(0xA7FC98), CRGB(0xE1FFD9)});
    AddColorPalette("Warm" /*"Warm"*/, {/*CRGB(0xFCDEBB),*/ CRGB(0xFAC589), CRGB(0xD94709), CRGB(0xA80022), CRGB(0x8C001C)});
    AddColorPalette("Pastell 1" /*"Pastel1"*/, {CRGB(0x3FB8AF), CRGB(0x7FC7AF), CRGB(0xDAD8A7), CRGB(0xFF9E9D), CRGB(0xFF3D7F)});  // Exclude?
    AddColorPalette("Pastell 2" /*"Pinkish2"*/, {CRGB(0xEB2A73), CRGB(0xFF619A), CRGB(0xFF94BB), CRGB(0xFFC7DB), CRGB(0xFFE8F0)}); // Exclude?
    AddColorPalette("Girlie" /*"Girlish1"*/, {CRGB(0x009999), CRGB(0x8E0039), CRGB(0xDB0058), CRGB(0xFF7400), CRGB(0xFFC700)});
    AddColorPalette("Natur 1" /*"Nature2"*/, {CRGB(0xBF2A23), CRGB(0xA6AD3C), CRGB(0xF0CE4E), CRGB(0xCF872E), CRGB(0x8A211D)});
    AddColorPalette("Natur 2" /*"Nature3"*/, {CRGB(0xFCDEBB), CRGB(0xFAC589), CRGB(0xD94709), CRGB(0xA80022), CRGB(0x8C001C)});
    AddColorPalette("Ozean" /*"Ocean2"*/, {CRGB(0xCFF09E), CRGB(0xA8DBA8), CRGB(0x79BD9A), CRGB(0x3B8686), CRGB(0x0B486B)}); // Exclude?

    // Christmas
    AddColorPalette("Weihnachten 1" /*"Christmas1"*/, {CRGB(0x941F1F), CRGB(0xCE6B5D), CRGB(0xFFEFB9), CRGB(0x7B9971), CRGB(0x34502B), CRGB(0x7A7E5F)}); // Exclude?
    AddColorPalette("Weihnachten 2" /*"Christmas2"*/, {CRGB(0x9E3333), CRGB(0xAB6060), CRGB(0xD4D1A5), CRGB(0x7BBD82), CRGB(0x3D9949), CRGB(0x7A7E5F)}); // Exclude?
    AddColorPalette("Weihnachten 3" /*"Christmas4"*/, {/*CRGB::Black,*/ CRGB::DarkViolet, CRGB::DarkRed, /*CRGB::Black,*/ CRGB::DarkOrange, CRGB::DarkGreen});
    AddColorPalette("Weihnachten 4" /*"Christmas8"*/, {/*CRGB::Black,*/ CRGB::DarkRed, CRGB::DarkOrange, CRGB::DarkRed, CRGB::DarkGreen});

    // Fires
    AddColorPalette("Kaminfeuer" /*"CozyFire3"*/, {CRGB::DarkRed, CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::DarkOrange});
}
#pragma endregion
