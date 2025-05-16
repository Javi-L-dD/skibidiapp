#include "manager.h"
#include "serialmanager.h"

using namespace std;

int cont = 0;

manager::manager() {
    // Ya no es necesario hacer new, ya que SerialManager es miembro directo de la clase
}

manager::~manager() {
    closePort();  // Cerramos el puerto cuando destruimos el objeto
}

// Usamos la instancia de serialManager para abrir el puerto
bool manager::openPort(const QString &portName) {
    return serialManager.openPort(portName);
}

// Usamos la instancia de serialManager para cerrar el puerto
void manager::closePort() {
    serialManager.closePort();
}

bool manager::checkPort() {
    return serialManager.checkPortStatus(); // Comprobamos el estado del puerto
}

bool manager::sendWritePacket(int index, const vector<uint8_t>& data) {
    vector<uint8_t> packet = serialManager.createWritePacket(index, data);  // Creamos el paquete de escritura
    return serialManager.sendData(packet);  // Usamos la función que envía el paquete
}

vector<uint8_t> manager::createReadPacket(int index) {
    return serialManager.createReadPacket(index);  // Devolvemos el paquete de lectura
}

// Función para enviar datos a través de SerialManager
bool manager::sendData(const vector<uint8_t>& data) {
    return serialManager.sendData(data);
}

// Función para recibir datos a través de SerialManager
vector<uint8_t> manager::receiveData() {

    // Para casos de prueba
    /*vector<uint8_t> packet = serialManager.readTestData(cont);
    packet.erase(packet.begin() + 5);
    packet.erase(packet.begin() + 6);
    ++cont;
    if (cont >= 4) {
        cont = 0;
    }
    return packet;*/

    return serialManager.readData();
}

bool manager::validateCRC(const vector<uint8_t>& response) {

    // Extraer los últimos dos bytes como el CRC recibido
    uint8_t crcHigh = response[response.size() - 2];  // MSB
    uint8_t crcLow = response[response.size() - 1];   // LSB
    uint16_t receivedCRC = (crcHigh << 8) | crcLow;   // Formar el CRC recibido en Big Endian

    // Calcular el CRC de los datos (sin los bytes CRC).
    uint16_t calculatedCRC = serialManager.crc16Modbus(response);

    // Comparar el CRC calculado con el CRC recibido
    if (calculatedCRC == receivedCRC) {
        // Si el CRC es válido, imprimir los CRC calculado y recibido.
        cout << "CRC Calculated: 0x" << hex << calculatedCRC << endl;
        cout << "CRC Received: 0x" << hex << receivedCRC << endl;
        return true;  // CRC es válido
    } else {
        // Si el CRC no es válido, imprimir un mensaje de error.
        cout << "CRC is not valid." << endl;
        return false;  // CRC no válido
    }
}
