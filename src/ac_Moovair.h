#ifndef _AC_MOOVAIR_H
#define _AC_MOOVAIR_H

#include <Arduino.h>

#define LG_ENERGY_SAVING_ON     0x1004
#define LG_ENERGY_SAVING_OFF    0x1005
#define LG_JET_ON               0x1008
#define LG_WALL_SWING_ON        0x1314
#define LG_WALL_SWING_OFF       0x1315
#define LG_SWING_ON             0x1316  // not verified, for AKB73757604
#define LG_SWING_OFF            0x1317  // not verified, for AKB73757604
#define LG_TIMER_ON             0x8000  // relative minutes in lower nibbles
#define LG_TIMER_OFF            0x9000  // relative minutes in lower nibbles
#define LG_SLEEP                0xA000  // relative minutes in lower nibbles
#define LG_CLEAR_ALL            0xB000  // Timers and sleep
#define LG_POWER_DOWN           0xC005
#define LG_LIGHT                0xC00A
#define LG_AUTO_CLEAN_ON        0xC00B
#define LG_AUTO_CLEAN_OFF       0xC00C

#define LG_COMMAND_OFF          '0'
#define LG_COMMAND_ON           '1'
#define LG_COMMAND_SWING        's'
#define LG_COMMAND_AUTO_CLEAN   'a'
#define LG_COMMAND_FAN_SPEED    'f'
#define LG_COMMAND_TEMPERATURE  't'
#define LG_COMMAND_TEMPERATURE_PLUS '+'
#define LG_COMMAND_TEMPERATURE_MINUS '-'
#define LG_COMMAND_MODE         'm'
#define LG_COMMAND_SLEEP        'S'
#define LG_COMMAND_TIMER_ON     'T'
#define LG_COMMAND_TIMER_OFF    'O'

#define AC_MODE_COOLING         'c'
#define AC_MODE_DEHUMIDIFIYING  'd'
#define AC_MODE_FAN             'f'
#define AC_MODE_AUTO            'a'
#define AC_MODE_HEATING         'h'

#define MOOVAIR_ADDRESS_STATE  0xA1
#define MOOVAIR_ADDRESS_COMMAND 0xA2
#define MOOVAIR_ADDRESS_FOLLOW  0xA4

#define MOOVAIR_COMMAND_SWING   0x2FFFFFF

#define MOOVAIR_MODE_COOL       0
#define MOOVAIR_MODE_DRY        1
#define MOOVAIR_MODE_AUTO       2
#define MOOVAIR_MODE_HEAT       3
#define MOOVAIR_MODE_FAN        4
#define MOOVAIR_FANSPEED_LOW    1
#define MOOVAIR_FANSPEED_MED    2
#define MOOVAIR_FANSPEED_HIGH   3
#define MOOVAIR_FANSPEED_AUTO   4

union MoovairState {
    uint32_t raw;
    struct {
        uint8_t OnTimerDuration : 7;
        uint8_t Unknown4 : 1;
        uint8_t OffTimerDuration : 7;
        uint8_t Unknown3 : 1;
        uint8_t Temperature : 4; // F: temp - 62, C: temp - 17
        uint8_t Unknown2 : 1; // Only 1 in fan mode
        uint8_t Fahrenheit : 1;
        uint8_t Unknown1 : 2; // Always 1
        uint8_t Mode : 3;
        uint8_t FanSpeed : 3;
        uint8_t Sleep : 1;
        uint8_t Power : 1;
    };
};

union MoovairFollowMe {
    uint32_t raw;
    struct {
        uint8_t SensorTemperature : 8; // F: temp - 63, C: just the temp
        uint8_t Unknown3 : 6; // always 0x3F
        uint8_t Enabled : 1;
        uint8_t Start : 1; // When 0, doesn't beep
        uint8_t Temperature : 4; // F: temp - 62, C: temp - 17
        uint8_t Unknown2 : 1; // Only seems to be 1 in fan mode
        uint8_t Fahrenheit : 1;
        uint8_t Unknown1 : 2; // Always 1
        uint8_t Mode : 3;
        uint8_t FanSpeed : 3;
        uint8_t Sleep : 1;
        uint8_t Power : 1;
    };
};

class Moovair {
public:
    Moovair();
    bool sendCommandAndParameter(char aCommand, int aParameter);
    void printMenu(Print *aSerial);
    void sendState();
    void sendFollowMe();
    static void PrintState(MoovairState state, Print *serial);
    static MoovairState GetDefaultState();
    static uint8_t calcChecksum(const uint8_t address, const uint32_t command);
    
    MoovairState CurrentState;
};

#endif
#pragma once
