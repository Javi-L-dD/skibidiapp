#ifndef MANAGER_H
#define MANAGER_H

#include <vector>
#include <stdint.h> // Para uint8_t y uint16_t
#include "serialmanager.h"
#include <iostream>

// Ignorar warnings, las bibliotecas son usadas en el source file (.cpp), no las reconoce como en uso porque no se usan en el propio header (.h)

using namespace std;

class manager {
public:
    manager(); // Constructor
    ~manager(); // Destructor

    // Public para poder acceder desde mainwindow
    bool openPort(const QString &portName); // Para llamar al backend y que abra el puerto
    void closePort(); // Para llamar al backend y que cierre el puerto
    bool checkPort(); // Para conectar con el backend y comprobar el estado del puerto (monitorización continua)
    bool sendWritePacket(int index, const vector<uint8_t>& data); // Enviar paquetes de escritura
    vector<uint8_t> createReadPacket(int index); // Crear paquetes de lectura
    bool sendData(const vector<uint8_t>& data); // Enviar datos
    vector<uint8_t> receiveData(); // Recibir datos
    bool validateCRC(const vector<uint8_t>& response); // Nueva función para validar CRC

private:
    SerialManager serialManager;  // Instancia de SerialManager para manejar la comunicación serial
};

#endif // MANAGER_H
