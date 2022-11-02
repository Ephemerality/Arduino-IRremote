#ifndef _IR_CODESET21_HPP
#define _IR_CODESET21_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

//#define SEND_CODESET21  1 // for testing
//#define DECODE_CODESET21  1 // for testing

// LSB first, 1 start bit + 16 bit address + 8 bit command + 1 stop bit.
#define CODESET21_ADDRESS_BITS      16 // 16 bit address
#define CODESET21_COMMAND_BITS      32 // Command

#define CODESET21_BITS              84 // (CODESET21_ADDRESS_BITS + CODESET21_COMMAND_BITS)
#define CODESET21_UNIT              610

#define CODESET21_HEADER_MARK       9100
#define CODESET21_HEADER_SPACE      4500

#define CODESET21_BIT_MARK          CODESET21_UNIT
#define CODESET21_ONE_SPACE         1750
#define CODESET21_ZERO_SPACE        610

//+=============================================================================
//
void IRsend::sendCodeset21(uint16_t aAddress, uint32_t aCommand) {
    // Not supported
}

//+=============================================================================
//
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeCodeset21() {

    // Check we have the right amount of data (28). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != (2 * CODESET21_BITS) + 4) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "space"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], CODESET21_HEADER_MARK) || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], CODESET21_HEADER_SPACE)) {
        IR_DEBUG_PRINT("Codeset21: ");
        IR_DEBUG_PRINTLN("Header mark or space length is wrong");
        return false;
    }

    // Read address
    if (!decodePulseDistanceData(32, 3, CODESET21_BIT_MARK, CODESET21_ONE_SPACE, CODESET21_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Codeset21: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    uint32_t tCommand = decodedIRData.decodedRawData;

    // Skip the extra crap
    decodePulseDistanceData(32, (CODESET21_COMMAND_BITS * 2) + 3, CODESET21_BIT_MARK, CODESET21_ONE_SPACE, CODESET21_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST);

    // Read what we'll use as an "address"
    if (!decodePulseDistanceData(20, (CODESET21_COMMAND_BITS * 2) + 3 + 64, CODESET21_BIT_MARK, CODESET21_ONE_SPACE, CODESET21_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Codeset21: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    uint16_t tAddress = decodedIRData.decodedRawData;

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * CODESET21_BITS)], CODESET21_BIT_MARK)) {
        IR_DEBUG_PRINT(F("Codeset21: "));
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST;
    decodedIRData.command = 0;
    decodedIRData.decodedRawData = tCommand;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = CODESET21_BITS;
    decodedIRData.protocol = CODESET21;

    return true;
}
#endif // _IR_CODESET21_HPP
#pragma once
