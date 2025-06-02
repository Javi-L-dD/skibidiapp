#include "serialmanager.h"
#include "values.h"
#include <iostream>

using namespace std;

SerialManager::SerialManager() {
    // El objeto QSerialPort ya está creado y persistente
}

SerialManager::~SerialManager() {
    closePort();
}

bool SerialManager::openPort(const QString& portName) {

    if (serial.isOpen()) {
        closePort(); // Si ya estaba abierto primero cerramos antes de reiniciar la conexión
    }

    // Esta es toda la configuración necesaria para el puerto según manual
    serial.setPortName(portName); // Que coincida el nombre que aparece en la lista con el que le asignamos al puerto
    serial.setBaudRate(QSerialPort::Baud115200); // Velocidad a la que trasnmiten los datos en bits por segundo (115200)
    serial.setDataBits(QSerialPort::Data8); // Bits de datos por trama (9)
    serial.setParity(QSerialPort::NoParity); // No hay paridad, no manda el bit adicional en los paquetes
    serial.setStopBits(QSerialPort::OneStop); // 1 bit de parada al final de los datos trasmitidos
    serial.setFlowControl(QSerialPort::NoFlowControl); // Sin controlo de flujo, no se satura el buffer

    // Necesitamos poder hacer operaciones de lectura y escritura
    if (!serial.open(QIODevice::ReadWrite)) {
        cerr << "Error when trying to open port.";
        return false;
    }

    cout << "Port " << portName.toStdString() << " succesfully opened.\n";
    return true;
}

void SerialManager::closePort() {

    // Cerrar el puerto si no se ha cerrado ya (por errores o desconexiones repentinas)
    if (serial.isOpen()) {
        serial.close();
        cout << "Port succesfully closed.\n" << flush;
    }
}

bool SerialManager::checkPortStatus() {

    // Monitorizamos el puerto y si salta un error o deja de poder ser R/W (siempre lo es en el sensor) es que se ha desconectado
    if (serial.isOpen() && (!serial.isWritable() || !serial.isReadable()|| serial.error() == QSerialPort::ResourceError)) {
        return false;
    }
    return true;
}

uint16_t SerialManager::crc16Modbus(const vector<uint8_t>& data) {

    // Hay que quitar los 2 ultimos bytes, que seran el crc, para calcularlo bien sobre el resto de bytes
    vector<uint8_t> dataWithoutDefaultCRC(data.begin(), data.end() - 2);

    // Función para calcular el crc correcto según el paquete
    uint16_t crc = 0xFFFF;
    for (uint8_t byte : dataWithoutDefaultCRC) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool SerialManager::sendData(const vector<uint8_t>& dataReceived) {

    // Recibe enteros (decimales)
    if (!serial.isOpen()) {
        cerr << "Error: port is not open.\n";
        return false;
    }

    // Calcular CRC16 en big endian
    uint16_t crc = crc16Modbus(dataReceived);
    vector<uint8_t> completeData = dataReceived;

    completeData.push_back((crc >> 8) & 0xFF);  // MSB
    completeData.push_back(crc & 0xFF);         // LSB

    // Imprimir los datos que se van a enviar byte a byte
    cout << "Sending data: ";
    for (size_t i = 0; i < dataReceived.size(); ++i) {
        cout << "0x" << hex << uppercase << (int)dataReceived[i] << " ";
    }
    cout << dec << endl;  // Regresar a formato decimal para el resto de los logs

    // Enviar datos
    QByteArray dataToSend(reinterpret_cast<const char*>(dataReceived.data()), dataReceived.size());
    if (serial.write(dataToSend) == -1) {
        cerr << "Error when writing into serial port.\n";
        return false;
    }

    if (!serial.waitForBytesWritten(1000)) {
        cerr << "Timeout when writing into serial port.\n";
        return false;
    }

    return true;
}

vector<uint8_t> SerialManager::readData() {

    // Variable auxiliar donde iremos guardando los datos
    vector<uint8_t> data;

    if (!serial.isOpen()) {
        cerr << "Error: port is not open.\n";
        return data;
    }

    // Timeout al segundo sin recibir respuesta
    if (!serial.waitForReadyRead(1000)) {
        cerr << "No response was received.\n";
        return data;
    }

    // Hay que darle un tiempo de espera para evitar que se corten paquetes
    // De esta forma no se reciben paquetes mas cortos cuyos bytes que faltan se añaden a otros paquetes que quedan mas largos
    QByteArray responseData = serial.readAll();
    while (serial.waitForReadyRead(100))
        responseData += serial.readAll();

    for (int i = 0; i < responseData.size(); ++i) {
        data.push_back(static_cast<uint8_t>(responseData[i]));
    }

    // Aqui van los casos de prueba
    /*
    QString portName = serial.portName();
    if (portName == "COM3") {

        data.push_back(0x40);  // Header
        data.push_back(0x80);  // Status (0x80 es OK)
        data.push_back(0x00);  // Datos
        data.push_back(0x00);
        data.push_back(0x00);  // Ejemplo de valor a recibir
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x00);

        // Calculamos el CRC sin incluir los bytes CRC
        uint16_t crc = crc16Modbus(data);  // Aquí calculamos el CRC con la función crc16Modbus

        // Big endian?
        // Agregar el CRC calculado al final del paquete (MSB y LSB)
        data.push_back((crc >> 8) & 0xFF);  // MSB del CRC
        data.push_back(crc & 0xFF);         // LSB del CRC

    }   else if (portName == "COM4") {

        data.push_back(0x40);  // Header
        data.push_back(0x80);  // Status (0x80 es OK)
        data.push_back(0x00);  // Primer byte
        data.push_back(0x00);  // Segundo byte (completamos con 0)
        data.push_back(0x00);  // Tercer byte (completamos con 0)
        data.push_back(0x7B);  // Cuarto byte
        data.push_back(0x00);
        data.push_back(0x00);

        // Calculamos el CRC sin incluir los bytes CRC
        uint16_t crc = crc16Modbus(data);  // Aquí calculamos el CRC con la función crc16Modbus

        // Big endian?
        // Agregar el CRC calculado al final del paquete (MSB y LSB)
        data.push_back((crc >> 8) & 0xFF);  // MSB del CRC
        data.push_back(crc & 0xFF);         // LSB del CRC

    }   else {

        data.push_back(0x40);  // Header
        data.push_back(0x80);  // Status (0x88 es NOTOK)
        data.push_back(0x00);  // Primer byte
        data.push_back(0x00);  // Segundo byte (completamos con 0)
        data.push_back(0x00);  // Tercer byte (completamos con 0)
        data.push_back(0x00);  // Cuarto byte
        data.push_back(0x00);
        data.push_back(0x10);

        // Calculamos el CRC sin incluir los bytes CRC
        uint16_t crc = crc16Modbus(data);  // Aquí calculamos el CRC con la función crc16Modbus

        // Big endian?
        // Agregar el CRC calculado al final del paquete (MSB y LSB)
        data.push_back((crc >> 8) & 0xFF);  // MSB del CRC
        data.push_back(crc & 0xFF);
    }
    */

    // Devolver el paquete completo
    return data;
}

/*
vector<uint8_t> SerialManager::readTestData(int readRegister) {

    vector<uint8_t> data;

    if (!serial.isOpen()) {
        cerr << "Error: serial port is not open.\n";
        return data;
    }

    data.push_back(0x40);  // Header
    data.push_back(0x80);  // Status (0x80 es OK)

    switch (readRegister) {
    case 0:
        data.push_back(0x00);  // Datos INTTIME
        data.push_back(0x01);
        data.push_back(0x60);
        data.push_back(0xAE);
        break;
    case 1:
        data.push_back(0x00);  // Datos INTPERIOD
        data.push_back(0x02);
        data.push_back(0xE8);
        data.push_back(0x0C);
        break;
    case 2:
        data.push_back(0x00);  // Datos GPOL
        data.push_back(0x00);
        data.push_back(0x09);
        data.push_back(0xF6);
        break;
    case 3:
        data.push_back(0x00);  // Datos OUTPUT
        data.push_back(0x00);
        data.push_back(0x00);
        data.push_back(0x0C);
        break;
    case 4:
        data.push_back(0x00);  // Datos MCK
        data.push_back(0x09);
        data.push_back(0x00);
        data.push_back(0x10);
        break;
    }

    data.push_back(0x00);  // Inicialmente a 0 para luego poder calcular el CRC
    data.push_back(0x00);

    // Calculamos el CRC sin incluir los bytes CRC
    uint16_t crc = crc16Modbus(data);  // Aquí calculamos el CRC con la función crc16Modbus
    data.push_back((crc >> 8) & 0xFF);  // MSB del CRC
    data.push_back(crc & 0xFF);         // LSB del CRC

    return data;
}
*/

// Los paquetes siguen una estructura predefinida y deben construirse byte a byte, siempre big endian

vector<uint8_t> SerialManager::createWritePacket(int index, const vector<uint8_t>& data) {

    vector<uint8_t> packet(WRITE_PACKET_SIZE, 0);  // Paquete de 12 bytes con ceros
    packet[0] = HEADER;  // Header

    // Command ID siempre 0x99 para escrituraa ver
    packet[1] = WRITE_COMMAND_ID;

    // Seleccionar la dirección correcta y expandirla a 4 bytes
    uint32_t address = static_cast<uint32_t>(index);

    // Direcciones en 4 bytes Big Endian
    packet[2] = (address >> 24) & 0xFF;  // Siempre será 0x00
    packet[3] = (address >> 16) & 0xFF;  // Siempre será 0x00
    packet[4] = (address >> 8) & 0xFF;   // Byte alto real
    packet[5] = address & 0xFF;          // Byte bajo real

    // Leer los 4 bytes de `data` correctamente en Big Endian
    packet[6] = data.size() > 0 ? data[0] : 0x00;
    packet[7] = data.size() > 1 ? data[1] : 0x00;
    packet[8] = data.size() > 2 ? data[2] : 0x00;
    packet[9] = data.size() > 3 ? data[3] : 0x00;

    // WRITE_REQUEST_PACKET.write_crc |= (crc >> 8) & 0xF | crc & 0xFF;
    // WRITE_REQUEST_PACKET.write_crc = __builtinswap16(crc16Modbus(packet));

    // Calcular CRC16 Big Endian
    uint16_t crc = crc16Modbus(packet);
    packet[10] = (crc >> 8) & 0xFF;  // MSB del CRC
    packet[11] = crc & 0xFF;         // LSB del CRC

    return packet;
}


vector<uint8_t> SerialManager::createReadPacket(int index) {

    vector<uint8_t> packet(READ_PACKET_SIZE, 0);  // Paquete de 8 bytes con ceros
    packet[0] = HEADER;  // Header
    packet[1] = READ_COMMAND_ID;  // Command ID siempre 0x90 para lectura

    // Seleccionar la dirección correcta y expandirla a 4 bytes
    uint32_t address = static_cast<uint32_t>(index);

    // Direcciones en 4 bytes Big Endian
    packet[2] = (address >> 24) & 0xFF;  // Siempre será 0x00
    packet[3] = (address >> 16) & 0xFF;  // Siempre será 0x00
    packet[4] = (address >> 8) & 0xFF;   // Byte alto real
    packet[5] = address & 0xFF;          // Byte bajo real

    // Calcular CRC16 Big Endian
    uint16_t crc = crc16Modbus(packet);
    packet[6] = (crc >> 8) & 0xFF;  // MSB del CRC
    packet[7] = crc & 0xFF;         // LSB del CRC

    return packet;
}
