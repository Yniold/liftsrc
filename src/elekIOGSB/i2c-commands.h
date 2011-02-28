#ifndef I2C_COMMANDS
#define I2C_COMMANDS

// I2C commands for the running AVR application
// commands are sent in this manner:

// [I2CADDR] [CMDBYTE] [NUMPARAMBYTES] [PARAM]

// set the Valve States accodingly
// takes 1 byte as parameter
// Valves are coded in Bit0-4
// other bits not used

#define CMD_SET_VALVEBITS           (0xE0)

#define CMD_SET_VALVEVOLT_HOLD      (0xE1)
#define CMD_SET_VALVEVOLT_SWITCH    (0xE2)

#define CMD_SET_LEDCURRENT          (0xE3)
#define CMD_SET_LEDSTATE            (0xE4)

#define CMD_GET_LED_CURRENT         (0xE5)
#define CMD_GET_LED_VOLTAGE         (0xE6)

#define CMD_GET_VALVE_CURRENT       (0xE7)
#define CMD_GET_VALVE_VOLTAGE       (0xE8)

#define CMD_GET_FIRMWARE_INFO       (0xEE)

#define CMD_JUMP_TO_BOOTLOADER      (0xEF)

// for bootloader
#define CMD_PING                    (0xF0)
#define CMD_SET_FLASH_ADDRESS       (0xF1)
#define CMD_TRANSFER_FLASH_DATA     (0xF2)
#define CMD_GET_BLOCK_CHECKSUM      (0xF3)
#define CMD_GET_FLASH_DATA          (0xF4)
#define CMD_EXIT_BOOTLOADER         (0xFE)

#endif

