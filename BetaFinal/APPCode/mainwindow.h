#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCoreApplication>
#include <QFileDialog>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QButtonGroup>
#include <QByteArray>
#include <QTimer>
#include <QDateTime>
#include <QTime>
#include <QRandomGenerator>
#include <QTextEdit>
#include <QDir>
#include <QFile>
#include <vector>
#include <bitset>
#include <string>
#include <iostream>
#include <sstream>

// Ignorar warnings, las bibliotecas son usadas en el source file (.cpp), no las reconoce como en uso porque no se usan en el propio header (.h)

using namespace std;

class manager;  // Declaramos el manejador aquí para usarlo más tarde

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // Constructor
    ~MainWindow(); //Destructor
    // Pública para agregar cout y cerr de otras clases a los logs
    static QTextEdit *globalLogBox;    // Manejador para unir los mensajes del sistema a los logs

private:
    manager *serialManager;               // Instancia de la clase manejador

    QStringList availablePorts;           // Lista de puertos disponibles
    QGridLayout *gridLayout;              // Layout en grid para las variables

    QComboBox *portSelector;              // Selector de puertos en menu dropdown
    QVector<QLineEdit*> readOnlyFields;   // Campos de solo lectura para mostrar valores
    QVector<QLineEdit*> writeFields;      // Campos de entrada para escribir valores
    QVector<QPushButton*> writeButtons;   // Botones para escribir los valores (mandar datos de los campos de escritura)

    QRadioButton *radioITR;               // Boton radio ITR (no interactuable por parte de usuario)
    QRadioButton *radioIWR;               // Boton radio IWR (no interactuable por parte de usuario)

    QPushButton *disconnectPortButton;    // Botón para desconectar el puerto actual
    QPushButton *updateButton;            // Botón para actualizar valores de los registros (campos de lectura)
    QPushButton *updatePortsButton;       // Botón para actualizar puertos
    QString selectedPort;                 // Puerto serie seleccionado actualmente (al que se conecta)
    QLineEdit *writeBOX;                  // Caja de escritura para la variable custom (dirección de memoria de la misma, en hex)
    QString customVariable;               // Variable para almacenar el valor de la Custom Variable (en decimal)

    QLineEdit *fpsBox;                    // Para mostrar los FPS
    QLineEdit *tframeBox;                 // Para mostrar el tframe (va a parte del resto de cajas del grid layout)
    QPushButton *hideLogs;                // Boton para ocultar o mostrar logs (ocultar el cuadro de texto)
    QPushButton *saveLogs;                // Boton para guardar los logs en un txt (en la subcarpeta EOLE_logs dentro del directorio de instalación de la app)
    QPushButton *clearLogs;               // Boton para borrar los logs (limpiar cuadro de texto)
    QTextEdit  *logs;                     // Caja para texto de los logs donde mostrar todo
    bool logsShown;                       // Comprobar si los logs se están mostrando o no para saber si ocultar o mostrar

    double MCK;                           // Guardar el MCK para operaciones internas de los cálculos de registros
    double MinTFrame;                     // Guardar el minimo Tframe para evitar pérdida de rendimiento
    double FPS;                           // Guardar variable con los FPS actuales
    int pixels;                           // Resolucion (operación de alto * ancho)
    int outputs;                          // Cantidad de outputs (Forzar a 4 si son 2 para evitar pérdida de rendimiento)

    vector<uint8_t> uint32ToBytes(uint32_t value);   // Función para convertir un uint32_t en un vector de 4 bytes
    bool sendWriteAndCheck(uint32_t address, const vector<uint8_t>& data); // Función para enviar un paquete y comprobar la respuesta
    void manageLogs();                    // Muestra u oculta los logs
    void clearAllLogs();                  // Maneja la limpieza de los logs
    void saveLogToFile();                 // Guarda los logs en un archivo default dentro de la carpeta de la app
    static void simpleMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg); // Gestionar los logs

    bool sendReadRequest(int address, vector<uint8_t>& response); // Función para mandar paquete de lectura
    QString formatResponseForDebug(const vector<uint8_t>& response); // Función para formatear la respuesta y poder debugearla (en los logs)
    bool sanitizeResponse(vector<uint8_t>& response); // Función para comprobar la respuesta del paquete
    bool validateAndExtractValue(const vector<uint8_t>& response, uint32_t& value); // Función para validar el paquete y extraer respuesta
    int getAddressFromIndex(int index);   // Función para obtener la dirección en función del index
    bool validateValueByType(int index, uint32_t value); // Función para validar el tipo de valor
    void updateReadOnlyField(int index, uint32_t value); // Función para actualizar los campos de solo lectura
    int decodeResolution(uint8_t resBits); // Funnción para obtener la resolución
    int decodeClkSource(uint8_t clkSRCbits); // Función para obtener el CLK
    int decodeMckDiv(uint8_t mckDIVbits); // Función para obtener los bits del MCK
    void calculateMCKValues(int clkSRC, int xclkDIV, int mckDIV); // Función para obtener el MCK
    void bitDecode(uint32_t data);        // Función que decodifica los datos en bits (debugging avanzado para registros custom)

    void storeCustomVariable();           // Función para almacenar la variable custom
    void updateAvailablePorts();          // Función para actualizar los puertos disponibles
    void openSerialPort();                // Función para abrir el puerto serie seleccionado
    void setControlsEnabled(bool enabled);// Habilitar/deshabilitar controles en función de la selección del puerto
    void writeVariable(int index);        // Función para escribir una variable en el registro correspondiente
    void updateValues();                  // Función para actualizar los valores en la UI (los registros)
    void writeOnInit();                   // Función para actualizar el period tras leer el time y ajustarlo al máximo rendimiento (fps)

    void readMCKRegister();               // Leemos el registro que determina ITR o IWR, el MCK
    void readOutputRegister();            // Leemos el registro que determina los outputs

    void onRadioITRToggled();             // Activamos los cambios si es modo ITR
    void onRadioIWRToggled();             // Activamos los cambios si es modo IWR

    bool validateCRC(const vector<uint8_t>& response);  // Funcion que llama al manejador para validar un CRC

    void handlePortSelection(int index);  // Función para manejar la selección de puertos
    void disconnectSerialPort();          // Función para desconectar el puerto serie
    void clearFields();                   // Limpia los campos al desconectar un puerto
    void monitorForcedDisconnects();      // Monitorizacion continua de fondo sobre desconexiones forzosas
};

#endif // MAINWINDOW_H
