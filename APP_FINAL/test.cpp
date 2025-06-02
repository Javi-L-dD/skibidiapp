// Funcion main para pruebas de la libreria y para ver ejemplos de uso de la misma
#include "eole_cmd.h"
#include <QList>
#include <QDebug>

// Tiene que estar comentado para no interferir cuando se debugee la app, que se ejecuta este main en lugar del main de la app

/*
int main(int argc, char *argv[]) {

    READ_REQUEST_PACKET readPacket;
    uint32_t readAddress = INT_TIME_ADDRESS;
    buildReadPacket(&readPacket, readAddress);
    // 0x6C 0xCF , CORRECTO

    WRITE_REQUEST_PACKET writePacket;
    uint32_t writeAddress = GPOL_ADDRESS;
    uint32_t writeData = 0x2B2A0;
    buildWritePacket(&writePacket, writeAddress, writeData);
    // 0xC8 0x76 , INCORRECTO

    RESPONSE_PACKET responsePacket;
    responsePacket.response_header = HEADER;
    responsePacket.response_status = PACKET_RESPONSE_OK;

    uint32_t testData = 0x3BD08;
    testData = TO_BIG_ENDIAN_32(testData);
    memcpy(&responsePacket.response_data_byte1, &testData, sizeof(testData));

    READ_REQUEST_PACKET aux;
    // 0x17 0x96 , INCORRECTO
    memcpy(&aux, &responsePacket, sizeof(responsePacket));
    crc16Modbus(&aux.read_header, READ_AND_RESPONSE_PACKET_SIZE);

    memcpy(&responsePacket.response_crc_byte1, &aux.read_crc_byte1, 2);

    uint32_t responseData = EMPTY;
    readResponsePacket(&responsePacket, &responseData);
    // Verificado correctamente CRC y status, se copian correctamente los datos

    READ_REQUEST_PACKET lecturaTINT;
    buildReadPacket(&lecturaTINT, INT_TIME_ADDRESS);
    // 0x6c 0xcf

    READ_REQUEST_PACKET lecturaTFRAME;
    buildReadPacket(&lecturaTFRAME, INT_PERIOD_ADDRESS);
    // 0x69 0xcf

    READ_REQUEST_PACKET lecturaGPOL;
    buildReadPacket(&lecturaGPOL, GPOL_ADDRESS);
    // 0xaa 0xce

    WRITE_REQUEST_PACKET escrituraunoTINT;
    buildWritePacket(&escrituraunoTINT, INT_TIME_ADDRESS, 0x2B2A0);
    // 0x41 0x40

    WRITE_REQUEST_PACKET escrituradosTINT;
    buildWritePacket(&escrituradosTINT, INT_TIME_ADDRESS, 0x1E6B8);
    // 0x8b 0x8e

    WRITE_REQUEST_PACKET escrituraunoTFRAME;
    buildWritePacket(&escrituraunoTFRAME, INT_PERIOD_ADDRESS, 0x3BD08);
    // 0xce 0x05

    WRITE_REQUEST_PACKET escrituradosTFRAME;
    buildWritePacket(&escrituradosTFRAME, INT_PERIOD_ADDRESS, 0x34616);
    // 0xf6 0xc6

    WRITE_REQUEST_PACKET escrituraunoGPOL;
    buildWritePacket(&escrituraunoGPOL, GPOL_ADDRESS, 0x708);
    // 0x6e 0x77

    WRITE_REQUEST_PACKET escrituradosGPOL;
    buildWritePacket(&escrituradosGPOL, GPOL_ADDRESS, 0x852);
    // 0xa5 0xf2

    return 0;
}
*/
