#ifndef __MOD_NFC_H__
#define __MOD_NFC_H__

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
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"
#include "src/NDEF/NfcAdapter.h"
#pragma endregion
// **************************************************

// **************************************************
// *** Variable and Constants Declarations
// **************************************************
#pragma region Variables and Constants
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
// *** NFC Helper Methods
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
    DEBUG_PRINTLN("NFC: NFC tag detected: " + String(isNfcCardDetected));

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

#endif