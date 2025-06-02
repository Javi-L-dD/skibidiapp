#ifndef VALUES_H
#define VALUES_H

#include <cstdint> // Sin esto no reconoce los uint8_t
using namespace std;

#define TITLE "EOLE Toolkit for Sensia" // Nombre
#define VERSION "V.1.6" // Versión actual (se va cambiando)

#define VARIABLES_QUANTITY 3 // Variables principales (TINT, FRAME, GPOL)
#define MCKREGISTER "0x028" // Dirección de memoria del registro con datos pertienentes al MCK
#define OUTPUTREGISTER "0x0B0" // Dirección de memoria del registro con datos pertinentes al output del sensor
#define ITR "ITR"
#define IWR "IWR"

#define HEADER 0x40 // Cabecera de los paquetes, siempre la misma (dirección del EOLE)
#define PACKET_RESPONSE_OK 0x80 // Comando de status OK
#define PACKET_RESPONSE_NOTOK 0x88 // Comando de status NOTOK

#define WRITE_COMMAND_ID 0x99 // Comando de escritura
#define READ_COMMAND_ID 0x90 // Comando de lectura

#define INT_TIME_ADDRESS 0x098 // Dirección de memoria correspondiente a TINT
#define INT_PERIOD_ADDRESS 0x094 // Dirección de memoria correspondiente a TFRAME
#define GPOL_ADDRESS 0x090 // Dirección de memoria correspondiente a GPOL

#define WRITE_PACKET_SIZE 12 // Tamaño de paquetes de escritura (y respuesta, ambos en bytes)
#define READ_PACKET_SIZE 8 // Tamaño de paquetes de lectura (en bytes)

#define TWO_VIDEO_OUTPUTS 2 // Si tiene 2 video outputs (tiene por default)
#define FOUR_VIDEO_OUTPUTS 4 // Si tiene 4 video outputs (hay que forzar que tenga)
#define CLK 20 // Variable para calcular mck
#define E6 1000000 // Exponente 6

#define OPERATION (36/3.5) // Operación interna del reloj

typedef struct WRITE_REQUEST_PACKET { // Estructura de los paquetes de escritura (12 x bytes, uint8_t)
    uint8_t write_header = HEADER; // Dirección de memoria, cabecera (1 x byte)
    uint8_t write_command_id = WRITE_COMMAND_ID; // Comando de id de escritura (1 x byte)
    uint32_t write_address; // Dirección de memoria a escribir (4 x bytes, uint8_t)
    uint32_t write_data; // Datos a escribir en memoria (4 x bytes, uint8_t)
    uint16_t write_crc; // CRC16 del paquete (2 x bytes, uint8_t)
}   WRITE_REQUEST_PACKET;

typedef struct READ_REQUEST_PACKET { // Estructura de los paquetes de lectura (12 x bytes, uint8_t)
    uint8_t read_header = HEADER; // Dirección de memoria, cabecera (1 x byte)
    uint8_t read_command_id = READ_COMMAND_ID;// Comando de id de lectura (1 x byte)
    uint32_t read_address; // Dirección de memoria a escribir (4 x bytes, uint8_t)
    uint16_t read_crc; // CRC16 del paquete (2 x bytes, uint8_t)
}   READ_REQUEST_PACKET;

typedef struct RESPONSE_PACKET { // Estructura de los paquetes de respuesta (12 x bytes, uint8_t)
    uint8_t response_header = HEADER; // Dirección de memoria, cabecera (1 x byte)
    uint8_t response_status; // ID comando de status OK ? NOTOK (1 x byte)
    uint32_t response_data; // Datos recibidos (4 x bytes, uint8_t)
    uint16_t response_crc; // CRC16 del paquete (2 x bytes, uint8_t)
}   RESPONSE_PACKET;

enum READONLYFIELDS { // Campos de solo lectura en el grid
    INTTIME = 0, INTPERIOD = 1, GPOL = 2, CUSTOM = 3
};

enum BITCASES { // Casos posibles de los bits (00, 01 y 10)
    ZERO = 0, ONE = 1, TWO = 2
};

enum MCKDIVVALUES { // Casos correspondientes a los bits 00, 01 y 10
    FIRST = 2, SECOND = 4, THIRD = 8
};

enum WINDOWMODEVALUES { // Resoluciones posibles en función de los bits 00, 01 y 10
    FIRSTRES = 640*512, SECONDRES = 512*512, THIRDRES = 640*480, FOURTHRES = -1
};

#endif // VALUES_H
