#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QtSerialPort/QSerialPort>
#include <vector>
#include <QString>
#include <cstdint>

using namespace std;

class SerialManager {
public:
    SerialManager(); // Constructor
    ~SerialManager(); // Destructor

    vector<uint8_t> createWritePacket(int index, const vector<uint8_t>& data); // Paquete de escritura de 12 bytes
    vector<uint8_t> createReadPacket(int index); // Paquete de lectura de 8 bytes
    bool openPort(const QString& portName); // Abrir puerto serial
    void closePort(); // Cerrar puerto serial
    bool checkPortStatus(); // Comprueba si el puerto sigue disponible
    bool sendData(const vector<uint8_t>& data); // Enviar paquetes al serial
    vector<uint8_t> readData(); // Leer paquetes recibidos por el serial
    vector<uint8_t> readTestData(int readRegister); // Casos de prueba
    uint16_t crc16Modbus(const vector<uint8_t>& data); // Calcular el crc en 2 bytes

private:
    QSerialPort serial; // Guardar el puerto serial
};

#endif // SERIALMANAGER_H
