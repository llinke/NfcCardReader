#ifndef __MOD_WIFI_H__
#define __MOD_WIFI_H__

// **************************************************
// *** Includes
// **************************************************
#pragma region Includes
#include "SerialDebug.h"
#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ESP8266mDNS.h>
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constamts  Declarations
// **************************************************
#pragma region Variables and Constants
const String wifiApName = "NfcScanner_AP";
const String wifiDnsName = "NfcScannerESP";
const int ConfigureAPTimeout = 300;
#pragma endregion
// **************************************************

// **************************************************
// *** WiFi Helper Methods
// **************************************************
#pragma region NFC Helper Methods
bool InitWifi(bool useWifiCfgTimeout = true, bool forceReconnect = false)
{
    DEBUG_PRINTLN("WIFI ------------------------------------------------------");

    DEBUG_PRINTLN("WiFi: setting host name to '" + wifiDnsName + "'...");
    WiFi.hostname(wifiDnsName.c_str());

    bool connected = (!forceReconnect && WiFi.status() == WL_CONNECTED);
    if (connected)
    {
        DEBUG_PRINTLN("WiFi: device '" + WiFi.hostname() + "' already connected...");
    }
    else
    {
        /*
        if (ledsInitialized)
        {
            FastLED.clear(true);
            fill_solid(leds, PIXEL_COUNT, CRGB::Blue);
            FastLED.show();
        }
        delay(2500);
        */
        blinkLeds(CRGB::Blue, 1000, 500);

        //WiFiManager
        WiFiManager wifiManager;
        if (forceReconnect)
        {
            wifiManager.resetSettings();
        }
        DEBUG_PRINTLN("WiFi Manager trying to connect...");
        if (useWifiCfgTimeout)
        {
            DEBUG_PRINTLN("You have " + String(ConfigureAPTimeout) + " seconds for configuration if required.");
            wifiManager.setConfigPortalTimeout(ConfigureAPTimeout);
        }
        //fetches ssid and pass from eeprom and tries to connect
        //if it does not connect it starts an access point with the specified name
        //and goes into a blocking loop awaiting configuration
        connected = wifiManager.autoConnect(wifiApName.c_str());
        //use this for auto generated name ESP + ChipID
        //wifiManager.autoConnect();
        //if you get here you have connected to the WiFi
    }

    DEBUG_PRINTLN(
        connected
            ? "Wifi is connected for device '" + WiFi.hostname() + "' with IP " + WiFi.localIP().toString() + "...yay!!!"
            : "!!! WIFI NOT CONNECTED !!!");
    /*
    if (ledsInitialized)
    {
        fill_solid(leds, PIXEL_COUNT, connected ? CRGB::Green : CRGB::Red);
        FastLED.show();
    }
    delay(2500);
    */
    blinkLeds(connected ? CRGB::Green : CRGB::Red, 1000, 500);

    return connected;
}
#pragma endregion
// **************************************************

#endif