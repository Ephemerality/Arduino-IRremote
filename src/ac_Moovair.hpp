#ifndef _AC_MOOVAIR_CPP
#define _AC_MOOVAIR_CPP
#include <Arduino.h>

#include "IRremoteInt.h"
#include "ac_Moovair.h"
#include "LongUnion.h"
#include "functions.hpp"

Moovair::Moovair() {
    CurrentState = Moovair::GetDefaultState();
}

void Moovair::PrintState(MoovairState moovairState, Print *serial) {
    serial->print("Power: ");
    serial->println(moovairState.Power);
    serial->print("Sleep: ");
    serial->println(moovairState.Sleep);
    serial->print("Fan speed: ");
    serial->println(moovairState.FanSpeed);
    serial->print("Mode: ");
    serial->println(moovairState.Mode);
    serial->print("Unknown1: ");
    serial->println(moovairState.Unknown1);
    serial->print("Unknown2: ");
    serial->println(moovairState.Unknown2);
    serial->print("Fahrenheit: ");
    serial->println(moovairState.Fahrenheit);
    serial->print("Temperature: ");
    uint8_t temperature = moovairState.Fahrenheit
      ? moovairState.Temperature + 62
      : moovairState.Temperature + 17;
    serial->println(temperature);
    serial->print("Off timer: ");
    uint8_t offTimerDuration = (moovairState.OffTimerDuration / 0.25) - 1;
    serial->println(offTimerDuration);
    serial->print("On timer: ");
    uint8_t onTimerDuration = (moovairState.OnTimerDuration / 0.25) - 1;
    serial->println(onTimerDuration);
}

MoovairState Moovair::GetDefaultState() {
    MoovairState state;
    state.raw = 0;
    state.Unknown1 = 1;
    state.Unknown2 = 0;
    state.Unknown3 = 1;
    state.OffTimerDuration = 0x7F;
    state.Unknown4 = 1;
    state.OnTimerDuration = 0x7F;
    return state;
}

void Moovair::printMenu(Print *aSerial) {
    aSerial->println();
    aSerial->println();
    aSerial->println(F("Type command and optional parameter without a separator"));
    aSerial->println(F("0 Off"));
    aSerial->println(F("1 On"));
    aSerial->println(F("s Swing <0 or 1>"));
    aSerial->println(F("a Self clean <0 or 1>"));
    aSerial->println(F("f Fan speed <0 to 2 or 3 for cycle>"));
    aSerial->println(F("t Temperature <18 to 30> degree"));
    aSerial->println(F("+ Temperature + 1"));
    aSerial->println(F("- Temperature - 1"));
    aSerial->println(F("m <c(ool) or a(uto) or d(ehumidifying) or h(eating) or f(an) mode>"));
    aSerial->println(F("S Sleep after <0 to 420> minutes"));
    aSerial->println(F("T Timer on after <0 to 1439> minutes"));
    aSerial->println(F("O Timer off after <0 to 1439> minutes"));
    aSerial->println(F("C Clear all timer and sleep"));
    aSerial->println(F("e.g. \"s1\" or \"t23\" or \"mc\" or \"O60\" or \"+\""));
    aSerial->println(F("No plausibility check is made!"));
    aSerial->println();
}

bool Moovair::sendCommandAndParameter(char aCommand, int aParameter) {
    // Commands without parameter
    switch (aCommand) {
    case LG_COMMAND_OFF:
        CurrentState.Power = 0;
        sendState();
        return true;

    case LG_COMMAND_ON:
        CurrentState.Power = 1;
        sendState();
        return true;

    case LG_COMMAND_TEMPERATURE_PLUS:
        if (18 <= CurrentState.Temperature && CurrentState.Temperature <= 29) {
            CurrentState.Temperature++;
            sendState();
        } else {
            return false;
        }
        return true;

    case LG_COMMAND_TEMPERATURE_MINUS:
        if (19 <= CurrentState.Temperature && CurrentState.Temperature <= 30) {
            CurrentState.Temperature--;
            sendState();
        } else {
            return false;
        }
        return true;

    }

    /*
     * Now the commands which require a parameter
     */
    if (aParameter < 0) {
        IR_DEBUG_PRINT(F("Error: Parameter is less than 0"));
        return false;
    }
    switch (aCommand) {

    case LG_COMMAND_MODE:
        CurrentState.Mode = aParameter + '0';
        sendState();
        break;

    case LG_COMMAND_SWING:
        return false;
        IR_DEBUG_PRINT(F("Send air swing="));
        IR_DEBUG_PRINTLN(aParameter);
        if (aParameter) {
            sendState();
        } else {
            sendState();
        }
        break;

    case LG_COMMAND_AUTO_CLEAN:
        return false;
        IR_DEBUG_PRINT(F("Send auto clean="));
        IR_DEBUG_PRINTLN(aParameter);
        if (aParameter) {
            sendState();
        } else {
            sendState();
        }
        break;

    case LG_COMMAND_FAN_SPEED:
        if (aParameter <= 4) {
            CurrentState.FanSpeed = aParameter;
            sendState();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TEMPERATURE:
        if (18 <= aParameter && aParameter <= 30) {
            CurrentState.Temperature = aParameter;
            sendState();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_SLEEP:
        return false;
        if (aParameter <= 420) {
            sendState();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TIMER_ON:
        // 1440 = minutes of a day
        if (aParameter <= 1439) {
            sendState();
        } else {
            return false;
        }
        break;

    case LG_COMMAND_TIMER_OFF:
        if (aParameter <= 1439) {
            sendState();
        } else {
            return false;
        }
        break;

    default:
        return false;
    }
    return true;
}

void Moovair::sendState() {
    IR_INFO_PRINT(F("Send code=0x"));
    IR_INFO_PRINT(CurrentState.raw, HEX);
    IR_INFO_PRINT(F(" | 0b"));
    IR_INFO_PRINT(MOOVAIR_ADDRESS_COMMAND, BIN);
    IR_INFO_PRINTLN(CurrentState.raw, BIN);

    IrSender.sendMoovair(MOOVAIR_ADDRESS_STATE, CurrentState.raw);
}

void Moovair::sendFollowMe() {
    MoovairFollowMe followMe;
    followMe.Power = 1;
    followMe.Mode = MOOVAIR_MODE_HEAT;
    followMe.FanSpeed = MOOVAIR_FANSPEED_HIGH;
    followMe.Unknown2 = 1;
    followMe.Temperature = 5;
    followMe.Start = 1;
    followMe.Enabled = 1;
    followMe.Unknown3 = 0x3F;
    followMe.SensorTemperature = 21;

    IR_INFO_PRINT(F("Send code=0x"));
    IR_INFO_PRINT(followMe.raw, HEX);
    IR_INFO_PRINT(F(" | 0b"));
    IR_INFO_PRINTLN(followMe.raw, BIN);

    IrSender.sendMoovair(MOOVAIR_ADDRESS_FOLLOW, followMe.raw);
}

uint8_t Moovair::calcChecksum(const uint8_t address, const uint32_t command) {
    uint8_t checksum = 0;
    checksum += Functions::reverse(address);
    for (int i = 24; i >= 0; i -= 8)
        checksum += Functions::reverse((command >> i) & 0xFF);
    return ~Functions::reverse(checksum - 1);
}

#endif