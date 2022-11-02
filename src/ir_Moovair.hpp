#ifndef _IR_MOOVAIR_HPP
#define _IR_MOOVAIR_HPP

#include <Arduino.h>
#include "ac_Moovair.hpp"

#include "IRremoteInt.h" // evaluates the DEBUG for IR_DEBUG_PRINT

#if ! defined(MICROS_PER_TICK)
#define MICROS_PER_TICK    50
#endif

//#define SEND_MOOVAIR  1 // for testing
//#define DECODE_MOOVAIR  1 // for testing
//==============================================================================
//
//
//                              MOOVAIR
//
//
//==============================================================================
#define MOOVAIR_ADDRESS_BITS      8 // 8 bit address
#define MOOVAIR_COMMAND_BITS      32 // Command
#define MOOVAIR_CHECKSUM_BITS     8 // 8 bit checksum

#define MOOVAIR_BITS              MOOVAIR_ADDRESS_BITS + MOOVAIR_COMMAND_BITS + MOOVAIR_CHECKSUM_BITS // Data negated and repeated
#define MOOVAIR_UNIT              550

#define MOOVAIR_HEADER_MARK       4500 // The length of the Header:Mark
#define MOOVAIR_HEADER_SPACE      4500  // The length of the Header:Space

#define MOOVAIR_BIT_MARK          MOOVAIR_UNIT        // The length of a Bit:Mark
#define MOOVAIR_ONE_SPACE         1650  // The length of a Bit:Space for 1's
#define MOOVAIR_ZERO_SPACE        MOOVAIR_UNIT        // The length of a Bit:Space for 0's

#define MOOVAIR_REPEAT_SPACE      5100

//+=============================================================================
//
void IRsend::sendMoovair(uint8_t aAddress, uint32_t aCommand) {
    uint8_t checksum = Moovair::calcChecksum(aAddress, aCommand);

    // Set IR carrier frequency
    enableIROut(38);

    // Header
    mark(MOOVAIR_HEADER_MARK);
    space(MOOVAIR_HEADER_SPACE);

    // Address
    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, aAddress,
        MOOVAIR_ADDRESS_BITS, PROTOCOL_IS_MSB_FIRST);

    // Command
    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, aCommand,
        MOOVAIR_COMMAND_BITS, PROTOCOL_IS_MSB_FIRST);

    // Checksum
    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, checksum,
        MOOVAIR_ADDRESS_BITS, PROTOCOL_IS_MSB_FIRST, SEND_STOP_BIT);

    space(MOOVAIR_REPEAT_SPACE);
    mark(MOOVAIR_HEADER_MARK);
    space(MOOVAIR_HEADER_SPACE);

    /// Send the complement of everything
    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, ~aAddress,
        MOOVAIR_ADDRESS_BITS, PROTOCOL_IS_MSB_FIRST);

    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, ~aCommand,
        MOOVAIR_COMMAND_BITS, PROTOCOL_IS_MSB_FIRST);

    sendPulseDistanceWidthData(MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_BIT_MARK, MOOVAIR_ZERO_SPACE, ~checksum,
        MOOVAIR_ADDRESS_BITS, PROTOCOL_IS_MSB_FIRST, SEND_STOP_BIT);
}

//+=============================================================================
//
/*
 * First check for right data length
 * Next check start bit
 * Next try the decode
 * Last check stop bit
 */
bool IRrecv::decodeMoovair() {
    if (decodedIRData.rawDataPtr->rawlen != 200)
        return false;

    // Check header "space"
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[1], MOOVAIR_HEADER_MARK) || !matchSpace(decodedIRData.rawDataPtr->rawbuf[2], MOOVAIR_HEADER_SPACE)) {
        IR_DEBUG_PRINT("Moovair: ");
        IR_DEBUG_PRINTLN("Header mark or space length is wrong");
        return false;
    }

    if (!decodePulseDistanceData(32, 3, MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    IR_DEBUG_PRINT(F("Decoded value: "));
    IR_DEBUG_PRINTLN(decodedIRData.decodedRawData);

    uint32_t first32 = decodedIRData.decodedRawData;

    if (!decodePulseDistanceData(16, 67, MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[3 + (2 * MOOVAIR_BITS)], MOOVAIR_BIT_MARK)) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Stop bit 1 mark length is wrong"));
        return false;
    }

    IR_DEBUG_PRINT(F("Decoded value: "));
    IR_DEBUG_PRINTLN(decodedIRData.decodedRawData);

    uint16_t next16 = decodedIRData.decodedRawData;

    // Check end of first batch
    if (!matchSpace(decodedIRData.rawDataPtr->rawbuf[100], MOOVAIR_REPEAT_SPACE)) {
        IR_DEBUG_PRINT("Moovair: ");
        IR_DEBUG_PRINTLN("End of batch space length is wrong");
        return false;
    }

    uint8_t tAddress = (first32 >> 24) & 0xFF; // first 8-bits
    uint8_t checksum = next16 & 0xFF;
    uint32_t tCommand = (first32 << 8) & 0xFFFFFF00;
    tCommand |= (next16 >> 8) & 0xFF;

    //Serial.println(tAddress, DEC);
    //Serial.println(tCommand, DEC);
    //Serial.print(tAddress, BIN);
    //for (int i = 31; i >= 0; i--)
    //    Serial.print(bitRead(tCommand,i));
     //Serial.println();

    if (Moovair::calcChecksum(tAddress, tCommand) != checksum) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINT(F("Bad checksum - got "));
        IR_DEBUG_PRINT(checksum);
        IR_DEBUG_PRINT(F(", expected "));
        IR_DEBUG_PRINTLN(Moovair::calcChecksum(tAddress, tCommand));
        return false;
    }

    IR_DEBUG_PRINT(F("Address: "));
    IR_DEBUG_PRINTLN(tAddress);
    IR_DEBUG_PRINT(F("Byte1: "));
    IR_DEBUG_PRINTLN((first32 >> 16) & 0xFF);
    IR_DEBUG_PRINT(F("Byte2: "));
    IR_DEBUG_PRINTLN((first32 >> 8) & 0xFF);
    IR_DEBUG_PRINT(F("Byte3: "));
    IR_DEBUG_PRINTLN(first32 & 0xFF);
    IR_DEBUG_PRINT(F("Byte4: "));
    IR_DEBUG_PRINTLN((next16 >> 8) & 0xFF);
    IR_DEBUG_PRINT(F("Checksum: "));
    IR_DEBUG_PRINTLN(checksum);
    IR_DEBUG_PRINT(F("Command: "));
    IR_DEBUG_PRINTLN(tCommand);

    if (!decodePulseDistanceData(32, 103, MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    uint32_t check32 = decodedIRData.decodedRawData;

    if (check32 != ~first32) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed (bad inverted first32)"));
        return false;
    }

    if (!decodePulseDistanceData(16, 167, MOOVAIR_BIT_MARK, MOOVAIR_ONE_SPACE, MOOVAIR_ZERO_SPACE, PROTOCOL_IS_MSB_FIRST)) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed"));
        return false;
    }

    uint16_t check16 = decodedIRData.decodedRawData;

    // Stop bit
    if (!matchMark(decodedIRData.rawDataPtr->rawbuf[199], MOOVAIR_BIT_MARK)) {
        IR_DEBUG_PRINT("Moovair: ");
        IR_DEBUG_PRINTLN(F("Stop bit mark length is wrong"));
        return false;
    }

    if (check16 != ~next16) {
        IR_DEBUG_PRINT(F("Moovair: "));
        IR_DEBUG_PRINTLN(F("Decode failed (bad inverted next16)"));
        return false;
    }

    decodedIRData.decodedRawData = tCommand;
    decodedIRData.command = 0;
    decodedIRData.address = tAddress;
    decodedIRData.numberOfBits = MOOVAIR_BITS;
    decodedIRData.protocol = MOOVAIR;
    decodedIRData.flags = IRDATA_FLAGS_IS_MSB_FIRST;

    return true;
}

#endif // _IR_MOOVAIR_HPP
#pragma once