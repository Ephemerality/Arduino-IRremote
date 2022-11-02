#ifndef _IR_CODESET8_HPP
#define _IR_CODESET8_HPP

#include <Arduino.h>

//#define DEBUG // Activate this for lots of lovely debug output from this decoder.
#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

//#define SEND_CODESET8  1 // for testing
//#define DECODE_CODESET8  1 // for testing

// LSB first, 1 start bit + 16 bit address + 8 bit command + 1 stop bit.
#define CODESET8_ADDRESS_BITS      16 // 16 bit address
#define CODESET8_COMMAND_BITS      32 // Command

#define CODESET8_BITS              (CODESET8_ADDRESS_BITS + CODESET8_COMMAND_BITS) // The number of bits in the protocol
#define CODESET8_UNIT              610

#define CODESET8_HEADER_MARK       9100
#define CODESET8_HEADER_SPACE      4100

#define CODESET8_BIT_MARK          CODESET8_UNIT
#define CODESET8_ONE_SPACE         1600
#define CODESET8_ZERO_SPACE        510

//+=============================================================================
//
void IRsend::sendCodeset8(uint16_t aAddress, uint32_t aCommand) {
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
bool IRrecv::decodeCodeset8() {

    // Check we have the right amount of data (28). The +4 is for initial gap, start bit mark and space + stop bit mark
    if (decodedIRData.rawDataPtr->rawlen != (2 * CODESET8_BITS) + 4) {
        // no debug output, since this check is mainly to determine the received protocol
        return false;
    }

    // Check header "space"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], CODESET8_HEADER_MARK) || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], CODESET8_HEADER_SPACE)) {
        IR_DEBUG_PRINT("Codeset8: ");
        IR_DEBUG_PRINTLN("Header mark or space length is wrong");
        return false;
    }

    // Read address
    if (!decodePulseDistanceData(16, 3, CODESET8_BIT_MARK, CODESET8_ONE_SPACE, CODESET8_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Codeset8: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    uint16_t tAddress = decodedIRData.decodedRawData & 0xFFFF;

    // Read command
    if (!decodePulseDistanceData(32, (CODESET8_ADDRESS_BITS * 2) + 3, CODESET8_BIT_MARK, CODESET8_ONE_SPACE, CODESET8_ZERO_SPACE, PROTOCOL_IS_LSB_FIRST)) {
        IR_DEBUG_PRINT(F("Codeset8: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * CODESET8_BITS)], CODESET8_BIT_MARK)) {
        IR_DEBUG_PRINT(F("Codeset8: "));
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    // Success
    decodedIRData.flags = IRDATA_FLAGS_IS_LSB_FIRST;
    decodedIRData.command = 0;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = CODESET8_BITS;
    decodedIRData.protocol = CODESET8;

    return true;
}
#endif // _IR_CODESET8_HPP
#pragma once
