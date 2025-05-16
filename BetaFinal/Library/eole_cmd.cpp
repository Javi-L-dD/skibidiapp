#include "eole_cmd.h" // Incluir el archivo header

// Aqui se implementa la funcionalidad de la libreria
// Cambiar extensión a .c y ya está adaptada para funcionar perfectamente en C

void buildReadPacket(READ_REQUEST_PACKET *readPacket, uint32_t address) { // Pasamos el puntero del paquete por referencia

    readPacket->read_header = HEADER; // Asignamos la cabecera
    readPacket->read_command_id = READ_COMMAND_ID; // Asignamos el ID del comando de lectura

    address = TO_BIG_ENDIAN_32(address); // Ajustamos a BIG ENDIAN la direccion del registro
    memcpy(&readPacket->read_address_byte1, &address, sizeof(address)); // Copiamos en memoria la direccion del registro

    crc16Modbus(&readPacket->read_header, READ_AND_RESPONSE_PACKET_SIZE); // Calculamos el CRC del paquete
}

void buildWritePacket(WRITE_REQUEST_PACKET *writePacket, uint32_t address, uint32_t data) { // Pasamos el puntero del paquete por referencia y los datos a enviar

    writePacket->write_header = HEADER; // Asignamos la cabecera
    writePacket->write_command_id = WRITE_COMMAND_ID; // Asignamos el ID del comando de escritura

    address = TO_BIG_ENDIAN_32(address); // Ajustamos a BIG ENDIAN la direccion del registro
    memcpy(&writePacket->write_address_byte1, &address, sizeof(address)); // Copiamos en memoria la direccion del registro

    data = TO_BIG_ENDIAN_32(data); // Ajustamos a BIG ENDIAN los datos a escribir
    memcpy(&writePacket->write_data_byte1, &data, sizeof(data)); // Copiamos en memoria los datos a enviar

    crc16Modbus(&writePacket->write_header, WRITE_PACKET_SIZE); // Calculamos el CRC del paquete
}

void readResponsePacket(RESPONSE_PACKET *responsePacket, uint32_t *data) { // Pasamos el puntero del paquete por referencia y la variable donde guardar los datos recibidos

    uint8_t status = EMPTY; // Creamos una variable de status para confirmar el estado de la respuesta
    memcpy(&status, &responsePacket->response_status, sizeof(status)); // Copiamos en memoria el status recibido

    uint16_t crcReceived = EMPTY; // Creamos una variable para guardar el CRC recibido del paquete de respuesta
    memcpy(&crcReceived, &responsePacket->response_crc_byte1, sizeof(crcReceived)); // Copiamos en memoria el CRC recibido

    READ_REQUEST_PACKET aux; // Creamos un paquete auxiliar
    memcpy(&aux, responsePacket, READ_AND_RESPONSE_PACKET_SIZE); // Copiamos en memoria todos los bytes del paquete de respuesta

    uint16_t crcCalculated = EMPTY; // Creamos una nueva variable para un CRC ideal
    crc16Modbus(&aux.read_header, READ_AND_RESPONSE_PACKET_SIZE); // Creamos el CRC que deberia tener un paquete de respuesta correcto
    memcpy(&crcCalculated, &aux.read_crc_byte1, sizeof(crcCalculated)); // Copiamos en memoria el CRC que se ha calculado

    if ((status == PACKET_RESPONSE_OK) && (crcReceived == crcCalculated)) { // Comparamos tanto el status como el CRC calculado con el obtenido para verificar la respuesta

        uint32_t tempData = EMPTY; // Si es correcta, guardamos una variable de datos vacia
        memcpy(&tempData, &responsePacket->response_data_byte1, sizeof(tempData)); // Copiamos en memoria de esta variable los datos recibidos

        tempData = TO_BIG_ENDIAN_32(tempData); // Pasamos a BIG ENDIAN los datos recibidos en la variable temporal
        *data = tempData; // Asignamos por referencia los datos recibidos ya procesados
    }

}

void crc16Modbus(uint8_t *pointer, uint8_t size) { // Pasamos por referencia el puntero, que comienza en el byte header del paquete, y el tamaño del paquete

    uint8_t *p = pointer; // Puntero auxiliar para no mover de posicion el original
    uint8_t aux = 0; // Auxiliar para iterar sobre sus valores y no los del puntero
    uint16_t crc = FULL_16; // Creamos los 2 bytes que tendran el CRC

    for (int i = 0; i < size - 2; i++) { // Aplicamos el algoritmo sobre todo el paquete quitando los 2 ultimos bytes, que seran el CRC
        aux = *p; // Iteramos sobre el aux
        crc ^= aux; // El valor donde comienza el puntero
        for (int j = 0; j < BYTE_SIZE; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
        p++; // Avanzamos a la siguiente direccion el puntero
    }

    crc = TO_BIG_ENDIAN_16(crc); // Hay que ponerlo en big endian
    memcpy(pointer + size - 2, &crc, sizeof(crc)); // Como ya hemos movido el puntero en el bucle, ya apunta a donde quedaria el byte 1 del CRC
}
