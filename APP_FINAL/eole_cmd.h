#ifndef EOLE_CMD_H
#define EOLE_CMD_H

// Esta libreria contiene las funcionalidades basicas del EOLE Toolkit sin necesidad de lanzar la APP

#include <stdint.h> // Para usar uint
#include <stdio.h> // Para usarlo en C
#include <string.h> // Para usar memcpy

// Ignorar warnings, las bibliotecas son usadas en el source file (.cpp), no las reconoce como en uso porque no se usan en el propio header (.h)

#define HEADER 0x40 // Siempre la misma direccion para el EOLE
#define EMPTY 0x0 // Byte vacio
#define FULL_16 0xFFFF // uint16_t lleno (2 bytes), todo a 1

#define PACKET_RESPONSE_OK 0x80 // Status response OK
#define PACKET_RESPONSE_NOTOK 0x88 // Status response NOT OK

#define WRITE_COMMAND_ID 0x99 // Comando para escritura
#define READ_COMMAND_ID 0x90 // Comando para lectura

#define INT_TIME_ADDRESS 0x098 // Direccion de memoria del registro INTEGRATION TIME / TINT
#define INT_PERIOD_ADDRESS 0x094 // Direccion de memoria del registro INTEGRATION PERIOD / TFRAME
#define GPOL_ADDRESS 0x090 // Direccion de memoria del registro GPOL
#define MCK_ADDRESS 0x028 // Direccion de memoria del registro CLOCK CONTROL / MCK (XCLK / MCK DIV y CLK SRC)
#define OUTPUT_ADDRESS 0x0B0 // Direccion de memoria del registro FPA (WINDOW MODE / RESOLUTION y FPA 4 VIDEO OUTPUTS)

#define WRITE_PACKET_SIZE 12 // Tamanyo en bytes de los paquetes de escritura
#define READ_AND_RESPONSE_PACKET_SIZE 8 // Tamanyo en bytes de los paquetes de lectura y los de respuesta
#define BYTE_SIZE 8 // Cantidad de bits en un byte

#define TO_BIG_ENDIAN_16(x) ( ((x) >> 8) | ((x) << 8) ) // Macro para poner en BIG ENDIAN un uint16_t (2 bytes)

#define TO_BIG_ENDIAN_32(x) ( \
    (((x) >> 24) & 0x000000FF) | \
    (((x) >> 8)  & 0x0000FF00) | \
    (((x) << 8)  & 0x00FF0000) | \
    (((x) << 24) & 0xFF000000) ) // Macro para poner en BIG ENDIAN un uint32_t (4 bytes)

typedef struct WRITE_REQUEST_PACKET { // Estructura de un paquete que se envia para solicitud de escritura (todo en hexadecimal y BIG ENDIAN)

    uint8_t write_header; // Cabecera (direccion)
    uint8_t write_command_id; // Identificador del comando de solicitud de escritura
    uint8_t write_address_byte1; // 4 bytes (uint32_t en total) que representan la direccion de memoria a escribir (registro)
    uint8_t write_address_byte2; // ""
    uint8_t write_address_byte3; // ""
    uint8_t write_address_byte4; // ""
    uint8_t write_data_byte1; // 4 bytes (uint32_t en total) que representan los datos a escribir en memoria
    uint8_t write_data_byte2; // ""
    uint8_t write_data_byte3; // ""
    uint8_t write_data_byte4; // ""
    uint8_t write_crc_byte1; // 2 bytes (uint16_t en total) que representan el CRC MODBUS para verificar el paquete
    uint8_t write_crc_byte2; // ""

}   WRITE_REQUEST_PACKET;

typedef struct READ_REQUEST_PACKET { // Estructura de un paquete que se envia para solicitud de lectura (todo en hexadecimal y BIG ENDIAN)

    uint8_t read_header; // Cabecera (direccion)
    uint8_t read_command_id; // Identificador del comando de solicitud de lectura
    uint8_t read_address_byte1; // 4 bytes (uint32_t en total) que representan la direccion de memoria a leer (registro)
    uint8_t read_address_byte2; // ""
    uint8_t read_address_byte3; // ""
    uint8_t read_address_byte4; // ""
    uint8_t read_crc_byte1; // 2 bytes (uint16_t en total) que representan el CRC MODBUS para verificar el paquete
    uint8_t read_crc_byte2; // ""

}   READ_REQUEST_PACKET;

typedef struct RESPONSE_PACKET { // Estructura para un paquete que se recibe como respuesta (todo en hexadecimal y BIG ENDIAN)

    uint8_t response_header; // Cabecera (direccion)
    uint8_t response_status; // Byte que informa si la operacion R/W fue satisfactoria o no (status)
    uint8_t response_data_byte1; // 4 bytes (uint32_t en total) que representan los datos recibidos del sensor
    uint8_t response_data_byte2; // ""
    uint8_t response_data_byte3; // ""
    uint8_t response_data_byte4; // ""
    uint8_t response_crc_byte1; // 2 bytes (uint16_t en total) que representan el CRC MODBUS para verificar el paquete
    uint8_t response_crc_byte2; // ""

}   RESPONSE_PACKET;

void buildReadPacket(READ_REQUEST_PACKET *readPacket, uint32_t address); // Función para construir paquetes de lectura a un registro
void buildWritePacket(WRITE_REQUEST_PACKET *writePacket, uint32_t address, uint32_t data); // Función para construir paquetes de lectura a un registro
void readResponsePacket(RESPONSE_PACKET *responsePacket, uint32_t *data); // Función para leer los datos de un paquete de respuesta
void crc16Modbus(uint8_t *pointer, uint8_t size); // Función para calcular el CRC de un paquete

#endif // EOLE_CMD_H

/*
class EOLE_CMD
{
public:
    EOLE_CMD();
};
*/
