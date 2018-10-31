#define ENABLE_SERIAL_DEBUG
#include "SerialDebug.h"

#if 0
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_SPI pn532spi(SPI, 10);
NfcAdapter nfc = NfcAdapter(pn532spi);
#else
#include <Wire.h>
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"
#include "src/NDEF/NfcAdapter.h"

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

const int sclPin = D1;
const int sdaPin = D2;

#endif

void setup(void)
{
  Serial.begin(115200);

  DEBUG_PRINTLN("Initializing I2C Wire");
  Wire.begin(sdaPin, sclPin);

  Wire.setClockStretchLimit(1600);

  ScanI2C();

  DEBUG_PRINTLN("Starting NDEF Reader");
  nfc.begin();
}

void loop(void)
{
  static int count = 0;
  count++;
  DEBUG_PRINTLN("\nScan a NFC tag (#" + String(count) + ")");

  if (nfc.tagPresent())
  {
    NfcTag tag = nfc.read();
    DEBUG_PRINTLN(tag.getTagType());
    DEBUG_PRINT("UID: ");
    DEBUG_PRINTLN(tag.getUidString());

    if (tag.hasNdefMessage()) // every tag won't have a message
    {

      NdefMessage message = tag.getNdefMessage();
      DEBUG_PRINT("\nThis NFC Tag contains an NDEF Message with ");
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
        DEBUG_PRINT("NDEF Record ");
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
  }
  delay(3000);
}

void ScanI2C()
{
  byte error, address;
  int nDevices;

  DEBUG_PRINTLN("Scanning I2C Bus...");
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
      DEBUG_PRINT("I2C device found at address 0x");
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
      DEBUG_PRINT("Unknown error at address 0x");
      if (address < 16)
      {
        DEBUG_PRINT("0");
      }
      DEBUG_PRINTLN(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    DEBUG_PRINTLN("No I2C devices found\n");
  }
  else
  {
    DEBUG_PRINTLN("Done.\n");
  }

  delay(2000);
}