#include "mainwindow.h"
#include "manager.h"
#include "values.h"

// Tanto la UI, como la clase auxiliar para los logs, como la lógica principal deben estar aquí, la lógica del serial port a parte
// Todo en una única ventana

using namespace std;

// INT_TIME es TTint
// INT_PERIOD es TFrame

// Esta clase se ocupa de enlazar con los logs todos los cout y cerr usados en otras clases (manejador y serialmanager)
// Debe declararse primero para poder ser usada en el constructor del mainwindow

class LogStream : public streambuf { // Hereda de streambuf
public:
    LogStream(QtMsgType type) : msgType(type) {} // Recibimos el tipo de mensaje en el constructor (un objeto tendra el cout y otro el cerr)

protected:
    virtual int overflow(int ch) override { // Cada vez que recibe algo el buffer interno y esta lleno, si es un salto de linea manda lo que tenga, si no lo añade al buffer
        if (ch == '\n') {
            flushBuffer();
        } else {
            buffer += static_cast<char>(ch);
        }
        return ch;
    }

    virtual int sync() override { // Cuando se vaya a limpiar el buffer lo fuerza que se vacie
        flushBuffer();
        return 0;
    }

private:
    void flushBuffer() {
        if (!buffer.isEmpty() && MainWindow::globalLogBox) { // Para el text log y cuando el buffer no este vacio, imprime
            QString prefix;
            QString color;

            switch (msgType) { // Segun si es un cout o un cerr, escoge el color
            case QtInfoMsg:  // stdout
                prefix = "[STDOUT]";
                color = "#1E90FF";  // Azul claro
                break;
            case QtCriticalMsg:  // stderr
                prefix = "[STDERR]";
                color = "#FF4500";  // Rojo anaranjado
                break;
            default:
                prefix = "[LOG]";
                color = "#000000";
                break;
            }

            // Con html, para que los caracteres especiales no rompan el mensaje
            QString htmlMessage = QString("<span style='color:%1;'>%2 %3</span>").arg(color).arg(prefix).arg(buffer.toHtmlEscaped());

            QMetaObject::invokeMethod(MainWindow::globalLogBox, [htmlMessage]() { // Lo añade al texto y manda la ultima linea
                MainWindow::globalLogBox->append(htmlMessage);
                MainWindow::globalLogBox->moveCursor(QTextCursor::End);
            }, Qt::QueuedConnection);

            buffer.clear(); // Limpia buffer y preparamos la siguiente linea

        }
    }

    QString buffer;
    QtMsgType msgType;
};

QTextEdit* MainWindow::globalLogBox = nullptr;  // Aquí hacemos que la variable sea usable

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) { // Constructor completo
    // Doble check del icono
    setWindowIcon(QIcon(":/icono.ico"));

    // Crear carpeta "EOLE_logs" en la ruta de ejecución
    QString logsDirPath = QCoreApplication::applicationDirPath() + "/EOLE_logs";
    QDir dir;
    if (!dir.exists(logsDirPath)) {
        dir.mkpath(logsDirPath);  // Crear solo si no existe
    }

    // Instanciamos la clase manejador que será la intermediaria con el backend
    serialManager = new manager;
    customVariable.clear(); // Limpiamos la variable custom

    // Creamos nuestro widget central al que añadiremos todo y nuestro main layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Establecer márgenes de 5% sobre el tamaño inicial (800x400)
    int marginX = width() * 0.05; // 5% horizontal
    int marginY = height() * 0.05; // 5% vertical
    mainLayout->setContentsMargins(marginX, marginY, marginX, marginY);
    mainLayout->setSpacing(8); // Reduce la separación entre layouts (8%)

    // Se calibra automáticamente la ventana
    stringstream ss;
    ss << TITLE << " " << VERSION;
    QString fulltitle = QString::fromStdString(ss.str()); // Título de la APP completo, nombre + version
    setWindowTitle(fulltitle);

    // Tamaño predeterminado y mínimo, el máximo sin límite (resolución completa)
    resize(800, 600);
    setMaximumSize(1000, 800);
    setMinimumSize(600, 400);

    // Layout de puertos
    QHBoxLayout *portLayout = new QHBoxLayout();

    // Drop-down para seleccionar el puerto
    portSelector = new QComboBox();
    portSelector->setPlaceholderText("Select COM USB port");

    // Botón para desconectar el puerto serie
    disconnectPortButton = new QPushButton("Disconnect port");
    disconnectPortButton->setEnabled(false);  // Inicialmente deshabilitado

    // Conectar la señal del botón con la funcion que los conecta
    connect(disconnectPortButton, &QPushButton::clicked, this, &MainWindow::disconnectSerialPort);

    // Botón de actualización de puertos
    updatePortsButton = new QPushButton("Update ports");
    // Conectarlo con la función que refresca puertos
    connect(updatePortsButton, &QPushButton::clicked, this, &MainWindow::updateAvailablePorts);

    // Conectar el dropdown para abrir el puerto elegido
    connect(portSelector, &QComboBox::currentTextChanged, this, &MainWindow::openSerialPort);

    // Proporcionados 60, 20 y 20, los tamaños de botones de puertos
    portLayout->addWidget(portSelector, 3);
    portLayout->addWidget(updatePortsButton, 1);
    portLayout->addWidget(disconnectPortButton, 1);

    mainLayout->addLayout(portLayout); // Ir añadiendo al mainlayout cada layout que se cree

    // Añadir QRadioButtons para los modos de integracion pero que no se pueda interactuar con ellos
    radioITR = new QRadioButton(ITR, this);
    radioITR->setAttribute(Qt::WA_TransparentForMouseEvents);
    radioIWR = new QRadioButton(IWR, this);
    radioIWR->setAttribute(Qt::WA_TransparentForMouseEvents);

    // Crear un QButtonGroup para asegurar que solo uno de los botones esté seleccionado
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(radioITR);
    buttonGroup->addButton(radioIWR);

    // Conectar los QRadioButtons a las funciones miembro
    connect(radioITR, &QRadioButton::toggled, this, &MainWindow::onRadioITRToggled);
    connect(radioIWR, &QRadioButton::toggled, this, &MainWindow::onRadioIWRToggled);

    // Centrar horizontalmente los radio buttons
    QHBoxLayout *radioLayout = new QHBoxLayout();
    radioLayout->addStretch();
    radioLayout->addWidget(radioITR);
    radioLayout->addSpacing(40);  // Espacio entre botones
    radioLayout->addWidget(radioIWR);
    radioLayout->addStretch();

    mainLayout->addLayout(radioLayout, Qt::AlignCenter); // Centrarlos

    // Layout de variables
    gridLayout = new QGridLayout();
    QStringList variableNames = {"TInt", "TFrame", "GPOL", "Custom register"};
    QStringList variableUnits = {"[MCK Period]", "[MCK Period]", "[mV]"};
    // Los nombres de las variables

    for (int i = 0; i < VARIABLES_QUANTITY + 1; ++i) {
        QLabel *label;
        QLineEdit *readOnlyBox;
        QLineEdit *writeBox;
        QPushButton *writeButton;

        // Adicionales para la fila de INT_PERIOD
        QLabel *registerLabel;

        if (i == VARIABLES_QUANTITY) {  // Nueva fila para "Custom Variable"
            // Usamos un campo vacío con el hint de "Custom variable"
            label = new QLabel("Custom register");
            readOnlyBox = new QLineEdit();
            readOnlyBox->setReadOnly(true);
            writeBox = new QLineEdit();
            writeButton = new QPushButton("Write register");

        }   else if (i == 1) {
            label = new QLabel(variableNames[i] + " " + variableUnits[i]);  // La fila de INT_PERIOD es distinta al resto, no puede ser modificada
            readOnlyBox = new QLineEdit();
            readOnlyBox->setReadOnly(true);

            registerLabel = new QLabel("TFrame (s)");
            tframeBox = new QLineEdit();
            tframeBox->setReadOnly(true);

            // Sin esto, al no corresponderse ningun boton con PERIOD, descuadra los indices que hay despues de este
            // Este boton no se usa ya que de momento no escribimos directamente en PERIOD
            writeBox = new QLineEdit();
            writeButton = new QPushButton("Hidden register (PERIOD)"); // Hay que añadir uno para no descuadrar los indices

        }   else {
            label = new QLabel(variableNames[i] + " " + variableUnits[i]);  // Usamos el nombre de la variable aquí
            readOnlyBox = new QLineEdit();
            readOnlyBox->setReadOnly(true);
            writeBox = new QLineEdit();
            writeButton = new QPushButton(QString("Write %1").arg(variableNames[i]));  // Botón con el nombre de la variable
        }

        // Añadir las 4 partes que forman cada línea para cada variable
        gridLayout->addWidget(label, i, 0);
        gridLayout->addWidget(readOnlyBox, i, 1);
        if (i != 1) {
            gridLayout->addWidget(writeBox, i, 2);
            gridLayout->addWidget(writeButton, i, 3);
        }   else {
            gridLayout->addWidget(tframeBox, i, 2);
            gridLayout->addWidget(registerLabel, i, 3, Qt::AlignCenter); // Alineado al centro
        }

        // Guardar los campos
        readOnlyFields.append(readOnlyBox);
        if (i != 1) {
            writeFields.append(writeBox);
            writeButtons.append(writeButton);
        }   else {
            readOnlyFields.append(tframeBox);
            writeFields.append(writeBox);
            writeButtons.append(writeButton);
        }

        // Conectamos el botón para escribir la variable correspondiente, salvo el de period que no se puede modificar por el usuario
        if (i != 1) {
            connect(writeButton, &QPushButton::clicked, this, [this, i]() { writeVariable(i); });
        }
    }

    // Quinta fila al grid layout con las FPS
    fpsBox = new QLineEdit();
    fpsBox->setReadOnly(true);
    QLabel *fpsLabel = new QLabel(" FPS");

    // Las FPS alineada
    gridLayout->addWidget(fpsBox, 4, 2);
    gridLayout->addWidget(fpsLabel, 4, 3, Qt::AlignCenter);

    // Márgenes adicionales para el grid layout, a parte de los márgenes de la app
    int gridMarginX = width() * 0.05; // 5% horizontal
    int gridMarginY = height() * 0.01; // 5% vertical
    gridLayout->setContentsMargins(gridMarginX, gridMarginY, gridMarginX, gridMarginY);

    // Añadimos el layout grid que contiene los campos al layout principal
    mainLayout->addLayout(gridLayout);

    // Crear la linea para añadirla variable custom
    writeBOX = new QLineEdit();
    QPushButton *writeBUTTON = new QPushButton("Select register");
    writeBOX->setPlaceholderText("Write custom register (Hex format: 0x00...)");

    // Hay que añadirlos tambien a los botones
    writeFields.append(writeBOX);
    writeButtons.append(writeBUTTON);

    // Crear un layout horizontal para alinear los widgets en una fila
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->addWidget(writeBOX, 4);
    hLayout->addWidget(writeBUTTON, 1);

    // Conectar el botón "Select this custom variable" al método de almacenamiento
    connect(writeBUTTON, &QPushButton::clicked, this, &MainWindow::storeCustomVariable);

    // Agregar el layout horizontal al layout principal
    mainLayout->addLayout(hLayout);

    // Layout para el boton update proporcionado
    QHBoxLayout *updateLayout = new QHBoxLayout();

    // Botón de actualización
    updateButton = new QPushButton("Update values");

    updateLayout->addStretch(4);         // 40%
    updateLayout->addWidget(updateButton, 2); // 20%
    updateLayout->addStretch(4);         // 40%

    mainLayout->addLayout(updateLayout);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::updateValues);

    // Deshabilitar controles inicialmente y poner el widget central
    setControlsEnabled(false);
    setCentralWidget(centralWidget);

    connect(portSelector, &QComboBox::currentIndexChanged, this, &MainWindow::handlePortSelection);

    // Layout para los logs con su boton alineado
    QVBoxLayout *logsLayout = new QVBoxLayout();
    QHBoxLayout *botonLogsLayout = new QHBoxLayout();

    hideLogs = new QPushButton("Show/Hide logs");
    connect(hideLogs, &QPushButton::clicked, this, &MainWindow::manageLogs);

    saveLogs = new QPushButton("Save logs");
    connect(saveLogs, &QPushButton::clicked, this, &MainWindow::saveLogToFile);

    clearLogs = new QPushButton("Clear logs");
    connect(clearLogs, &QPushButton::clicked, this, &MainWindow::clearAllLogs);

    // Añadimos espacio y alineamos el boton en proporciones de espacio y botones
    botonLogsLayout->addStretch(3);
    botonLogsLayout->addWidget(hideLogs, 1);
    botonLogsLayout->addStretch(1);
    botonLogsLayout->addWidget(saveLogs, 1);
    botonLogsLayout->addStretch(1);
    botonLogsLayout->addWidget(clearLogs, 1);
    botonLogsLayout->addStretch(3);

    // Creamos caja de texto para los logs, no se puede interactuar con ella
    logs = new QTextEdit();
    logs->setReadOnly(true);
    globalLogBox = logs;    // Unir el puntero
    qInstallMessageHandler(simpleMessageHandler);
    logsShown = true; // Inicialmente mostrados

    logsLayout->addLayout(botonLogsLayout);
    logsLayout->addWidget(logs);

    // Usamos la clase de arriba para logstream, una para salida estándar y otra para errores, que redirigen estas salidas al log
    static LogStream logStreamOut(QtInfoMsg);
    static LogStream logStreamErr(QtCriticalMsg);

    // Unimos el buffer de salida estándar y de salida de errores a las clases logstream
    cout.rdbuf(&logStreamOut);
    cerr.rdbuf(&logStreamErr);

    mainLayout->addLayout(logsLayout);

    // Detectar puertos disponibles solo al iniciar la aplicación
    updateAvailablePorts();

    // Crear e iniciar el timer para la función de monitoreo
    QTimer *disconnectTimer = new QTimer(this);
    connect(disconnectTimer, &QTimer::timeout, this, &MainWindow::monitorForcedDisconnects);
    disconnectTimer->start(200); // 200 ms, no hace falta un Thread porque consume muy muy poco y no interfiere con la app
}

MainWindow::~MainWindow() {
    // Destructor para borrar las instancias que creamos (la del manejador)
    disconnectSerialPort(); // Desconectar de forma segura antes de cerrar
    delete serialManager;
}

// Guardar los logs en un archivo
void MainWindow::saveLogToFile() {
    if (!globalLogBox) return;

    // No guardar nada si no hay texto
    if (globalLogBox->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Error", "Empty logs can not be saved.");
        return;
    }

    // Esto para elegir directorio donde guardar los logs abierto directamente donde la app
    /*QString selectedDir = QFileDialog::getExistingDirectory(this, "Select directory to save log", QCoreApplication::applicationDirPath());

    if (selectedDir.isEmpty()) return;  // Cancelado por el usuario

    // Crear subcarpeta "EOLE_logs" dentro del directorio elegido (NO EN USO, EN PRINCIPIO SE CREA DONDE SE GENERE LA APP)
    QString logsDirPath = selectedDir + "/EOLE_logs";*/

    QString logsDirPath = QCoreApplication::applicationDirPath() + "/EOLE_logs"; // Esto para guardarlos en la carpeta dentro de la app
    QDir dir;
    if (!dir.exists(logsDirPath)) {
        dir.mkpath(logsDirPath);  // Crear si no existe, de nuevo
    }

    // Crear nombre único con timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString fileName = QString("EOLE_logs_%1.txt").arg(timestamp);
    QString filePath = logsDirPath + "/" + fileName;

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << globalLogBox->toPlainText();
        file.close();

        QMessageBox::information(this, "Notification", "Logs saved to:\n" + filePath);
    } else {
        QMessageBox::warning(this, "Error", "Unable to save log to file.");
    }
}

void MainWindow::simpleMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (!globalLogBox) return;

    // Todos los tipos de notificaciones que hace el Qt
    QString prefix;
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG] "; break;
    case QtWarningMsg:  prefix = "[WARNING] "; break;
    case QtCriticalMsg: prefix = "[CRITICAL] "; break;
    case QtFatalMsg:    prefix = "[FATAL] "; break;
    case QtInfoMsg:     prefix = "[INFO] "; break;
    }

    // Diferente formato y color para cada uno de los distintos tipos
    QString finalMessage;
    switch (type) {
    case QtDebugMsg:    finalMessage = "<font color='gray'>[DEBUG] " + msg + "</font>"; break;
    case QtInfoMsg:     finalMessage = "<font color='blue'>[INFO] " + msg + "</font>"; break;
    case QtWarningMsg:  finalMessage = "<font color='orange'>[WARNING] " + msg + "</font>"; break;
    case QtCriticalMsg: finalMessage = "<font color='red'>[CRITICAL] " + msg + "</font>"; break;
    case QtFatalMsg:    finalMessage = "<font color='darkred'>[FATAL] " + msg + "</font>"; break;
    }

    // Scroll automatico al final
    QMetaObject::invokeMethod(globalLogBox, [finalMessage]() {

    // Si nos pasamos de 1000 lineas automaticamente borra las mas antiguas
    if (globalLogBox->document()->blockCount() > 1000) {
        QTextCursor cursor(globalLogBox->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();  // Eliminar salto de línea
    }

        globalLogBox->append(finalMessage);
        globalLogBox->moveCursor(QTextCursor::End);
    }, Qt::QueuedConnection);
}

// Gestionar el mostrar u ocultar los logs
void MainWindow::manageLogs() {
    // Si se están mostrando, se ocultan, y viceversa
    if (logsShown) {
        logs->hide();
        logsShown = false;
    }   else {
        logs->show();
        logsShown = true;
    }
}

// Limpiar el texto de logs
void MainWindow::clearAllLogs() {
    logs->clear();
    QMessageBox::information(this, "Notification", "Logs have been cleared.");
}

void MainWindow::storeCustomVariable() {
    // Obtener el texto ingresado
    QString customText = writeBOX->text();

    // Intentar convertir el texto a un número hexadecimal (base 16)
    bool ok;
    // Verificamos si la cadena tiene el prefijo "0x" y lo ignoramos si lo tiene
    if (customText.startsWith("0x", Qt::CaseInsensitive)) {
        // Convertir la parte hexadecimal de la cadena a un número entero
        customText = customText.mid(2);  // Quitar el prefijo "0x"
    }

    // Convertir a entero en base 16 (hexadecimal) a un número decimal
    uint32_t value = customText.toUInt(&ok, 16); // El "16" especifica que es hexadecimal

    if (ok) {
        // Si la conversión fue exitosa, almacenamos el valor en formato decimal
        customVariable = QString::number(value);  // Almacenamos como un QString decimal
        qDebug() << "Custom variable saved:" << customVariable;
    } else {
        // Si la conversión falla, mostramos un mensaje de error
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid hexadecimal number (e.g., 0x098).");
        qDebug() << "Custom variable input not valid";
    }
}

void MainWindow::updateAvailablePorts() {
    // Reiniciamos el puerto al que estamos conectado, por si no aparece
    QString currentPort = selectedPort;
    portSelector->clear();
    availablePorts.clear();  // Limpiar la lista global de puertos antes de actualizarla

    // Añadir a la lista los puertos que encuentre
    for (const QSerialPortInfo &portInfo : QSerialPortInfo::availablePorts()) {
        availablePorts.append(portInfo.portName());
    }
    portSelector->addItems(availablePorts);

    int puertos = availablePorts.length();

    // Le damos tiempo para que no bloquee la aparición de la ventana principal, ya que se invoca según se inicializa y si no solapa
    QTimer::singleShot(100, this, [this, puertos]() {
        // Crear y mostrar el popup de notificación con el número de puertos encontrados
        if (puertos > 0) {
            QMessageBox::information(this, "Ports Updated",
                                     QString("%1 port/s found.").arg(puertos),
                                     QMessageBox::Ok);
        } else {
            QMessageBox::information(this, "Ports Updated",
                                     "No ports found.",
                                     QMessageBox::Ok);
        }
        clearFields();
    });

    // Si el puerto seleccionado ya no está disponible, limpiar la selección
    if (portSelector->currentIndex() != -1) {
        if (!availablePorts.contains(currentPort)) {
            selectedPort.clear();
            disconnectSerialPort();
        }
    }
}

void MainWindow::openSerialPort() {
    // Escogemos el elegido en el dropdown menu
    selectedPort = portSelector->currentText();
    if (selectedPort.isEmpty()) {
        setControlsEnabled(false);
        return;
    }

    // Si no podemos conectarnos (puerto no disponible) mostrar el error
    if (!serialManager->openPort(selectedPort)) {
        QMessageBox::warning(this, "Error", "Serial port could not be opened.");
        setControlsEnabled(false);
        clearFields();
        disconnectSerialPort();
        return;
    }

    setControlsEnabled(true);
    clearFields(); // Limpiamos los campos
    updateValues(); // Actualiza valores al cambiar de puerto
}

void MainWindow::setControlsEnabled(bool enabled) {
    // Habilitar/deshabilitar los campos y botones en la UI según el estado de la conexión
    for (auto &field : writeFields) {
        field->setEnabled(enabled);
    }
    for (auto &button : writeButtons) {
        button->setEnabled(enabled);
    }
    updateButton->setEnabled(enabled);
}

void MainWindow::onRadioITRToggled() {
    // Si estamos en modo ITR, desactivar el IWR
    if (radioITR->isChecked()) {
        if (!radioIWR->isChecked()) {
            radioIWR->setChecked(false);
        }
    }
}

void MainWindow::onRadioIWRToggled() {
    // Si estamos en modo IWR, desactivar el ITR
    if (radioIWR->isChecked()) {
        if (!radioITR->isChecked()) {
            radioITR->setChecked(false);
        }
    }
}

vector<uint8_t> MainWindow::uint32ToBytes(uint32_t value) {
    // Adapta un valor de 4 bytes a un vector de 4 bytes en big endian
    return {
        static_cast<uint8_t>((value >> 24) & 0xFF),
        static_cast<uint8_t>((value >> 16) & 0xFF),
        static_cast<uint8_t>((value >> 8) & 0xFF),
        static_cast<uint8_t>(value & 0xFF)
    };
}

bool MainWindow::sendWriteAndCheck(uint32_t address, const vector<uint8_t>& data) {
    // Mandamos datos
    if (!serialManager->sendWritePacket(address, data)) {
        QMessageBox::warning(this, "Error", "Data could not be sent.");
        return false;
    }

    // Recibimos datos
    vector<uint8_t> response = serialManager->receiveData();
    if (response.empty()) {
        QMessageBox::warning(this, "Error", "No response received from the device.");
        return false;
    }

    // Feedback de la respuesta formateada
    qDebug() << "Response received after writing: "
             << QByteArray(reinterpret_cast<const char*>(response.data()), response.size()).toHex(' ');

    // Comprobamos el CRC
    if (!validateCRC(response)) {
        QMessageBox::warning(this, "Error", "Response does not have a valid CRC.");
        return false;
    }

    // Comprobamos que sea respuesta correcta (header 0x40 y status 0x80 OK)
    if (response[0] != HEADER || response[1] != PACKET_RESPONSE_OK) {
        QMessageBox::warning(this, "Error", "Device did not confirm writing value.");
        return false;
    }

    return true;
}

void MainWindow::writeOnInit() {
    // Primero leer el registro del reloj interno (masterclock), necesitamos los valores de sus bits
    readMCKRegister();

    // Obtenemos el texto del campo de INT_TIME y lo convertimos
    QString value = readOnlyFields[INTTIME]->text();
    if (value.isEmpty()) return;

    // Calculamos el nuevo valor del periodo en funcion del tiempo de integración
    int newPeriodValue = (MinTFrame * MCK) + ONE;
    vector<uint8_t> data = uint32ToBytes(newPeriodValue);

    // Enviamos el paquete
    if (!sendWriteAndCheck(INT_PERIOD_ADDRESS, data)) return;

    readOnlyFields[INTPERIOD]->setText(QString::number(newPeriodValue));
}

void MainWindow::writeVariable(int index) {
    // Si queremos escribir en el registro custom y no hay, volvemos
    if (index == VARIABLES_QUANTITY && customVariable.isEmpty()) {
        QMessageBox::warning(this, "Error", "No custom variable stored.");
        return;
    }

    // Obtenemos el valor correspondiente
    QString value = writeFields[index]->text();

    if (value.isEmpty()) return;

    bool ok;
    uint32_t decimalValue;

    if (index == VARIABLES_QUANTITY) {  // Custom register será en hexadecimal, los demás en decimal
        if (value.startsWith("0x", Qt::CaseInsensitive)) {
            value = value.mid(2);
        }

        // Comprobar que sea hexadecimal
        decimalValue = value.toUInt(&ok, 16);
        if (!ok) {
            QMessageBox::warning(this, "Error", "Invalid hex value (e.g., 0x1A or 1A).");
            return;
        }
    } else {
        // Comprobar que sea decimal
        decimalValue = value.toUInt(&ok, 10);
        if (!ok) {
            QMessageBox::warning(this, "Error", "Invalid decimal value.");
            return;
        }
    }

    // Validamos el tipo de dato
    if (!validateValueByType(index, decimalValue)) {
        return;
    }

    // Que nunca supere el TFrame al TInt, se descalibra
    if (index == INTPERIOD) {
        int intTime = readOnlyFields[INTTIME]->text().toInt();
        if (decimalValue < intTime) {
            QMessageBox::warning(this, "Error", "INT_PERIOD can't be less than INT_TIME.");
            return;
        }
    }

    // Dirección a escribir
    int address = (index < 3) ?
                  (index == INTTIME ? INT_TIME_ADDRESS :
                   index == INTPERIOD ? INT_PERIOD_ADDRESS :
                   GPOL_ADDRESS) :
                   customVariable.toInt();

    // Si en los registros custom elegimos Tint o Tframe, se gestiona
    if (customVariable.toInt() == INTTIME) { address = INTTIME; }
    if (customVariable.toInt() == INTPERIOD) { address = INTPERIOD; }

    auto data = uint32ToBytes(decimalValue);
    if (!sendWriteAndCheck(address, data)) return;

    // Pero lo tiene que meter en otro porque hay 1 mas, el de FPS
    // Lógica especial para escritura en campo de lectura
    if (index != INTTIME && index != INTPERIOD) {
        // Hay que compensar el índice porque el campo de tframe descuadra la alineación a partir del period
        if (index == VARIABLES_QUANTITY) {
            QString hexString = QString("0x%1").arg(decimalValue, 8, 16, QLatin1Char('0')).toUpper().replace("X", "x");
            readOnlyFields[++index]->setText(hexString);

            // Si el registro custom es Tint o Tframe hay que actualizar los campos y todos los valores que dependen de ambos
            if (customVariable.toInt() == INTTIME || customVariable.toInt() == INTPERIOD) { updateValues(); }
            // Si el registro custom es el GPOL, actualizar tambien el campo del GPOL
            if (customVariable.toInt() == GPOL_ADDRESS) { readOnlyFields[GPOL]->setText(QString::number(hexString.toInt())); }

        } else {
            readOnlyFields[++index]->setText(QString::number(decimalValue));
        }

    } else {
        readOnlyFields[index]->setText(QString::number(decimalValue));
    }

    // Lógica especial para INT_TIME
    if (index == INTTIME) {
        writeOnInit();
    }
}

bool MainWindow::sendReadRequest(int address, vector<uint8_t>& response) {
    vector<uint8_t> packet = serialManager->createReadPacket(address);

    // Mandamos el paquete
    if (!serialManager->sendData(packet)) {
        QMessageBox::warning(this, "Error", "No writing petition could be sent.");
        return false;
    }

    qDebug() << "Petition sent, awaiting answer...";

    // Leemos la respuesta
    response = serialManager->receiveData();
    if (response.empty()) {
        QMessageBox::warning(this, "Error", "No response was received.");
        return false;
    }

    qDebug() << "Response received:" << formatResponseForDebug(response);
    return true;
}

QString MainWindow::formatResponseForDebug(const vector<uint8_t>& response) {
    // Para ver en hexadecimal que todo el paquete sea correcto
    QString str;
    for (auto byte : response) {
        str += QString("0x%1 ").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
    }
    return str.trimmed();
}

bool MainWindow::sanitizeResponse(vector<uint8_t>& response) {
    // Ajustar el paquete de respuesta inicial y los posibles paquetes con bytes extra por la comunicación serial
    if (response.size() > 8) {
        auto it = std::find(response.begin(), response.end(), HEADER);
        if (it != response.end() && distance(it, response.end()) >= 8) {
            response = vector<uint8_t>(it, it + 8);
        } else {
            QMessageBox::warning(this, "Error", "Invalid or incomplete response.");
            return false;
        }
    }

    // Si despues de ajustar el paquete para evitar los paquetes "saturados" del serial, sigue sin ser correcto, salimos
    if (response.size() != 8 || response[0] != HEADER ||
        (response[1] != PACKET_RESPONSE_OK && response[1] != PACKET_RESPONSE_NOTOK)) {
        QMessageBox::warning(this, "Error", "Incorrect response format.");
        return false;
    }

    return true;
}

bool MainWindow::validateAndExtractValue(const vector<uint8_t>& response, uint32_t& value) {
    // Comprobar que la respuesta sea correcta y extraerla
    if (response[1] == PACKET_RESPONSE_OK && validateCRC(response)) {
        value = (response[5] | (response[4] << 8) | (response[3] << 16) | (response[2] << 24));
        return true;
    }

    QMessageBox::warning(this, "Error", "Response was not validated successfully.");
    return false;
}


void MainWindow::updateValues() {
    // Actualizar todos los valores (el registro custom solo si hay un registro escogido)
    int limit = customVariable.isEmpty() ? VARIABLES_QUANTITY - 1 : VARIABLES_QUANTITY;

    // Misma secuencia de operaciones para cada registro
    for (int i = 0; i <= limit; ++i) {
        int address = getAddressFromIndex(i);

        vector<uint8_t> response;
        if (!sendReadRequest(address, response)) return;
        if (!sanitizeResponse(response)) return;

        uint32_t value = 0;
        if (!validateAndExtractValue(response, value)) return;

        if (!validateValueByType(i, value)) return;

        updateReadOnlyField(i, value);

        // Cuando leamos el primer valor, también sacamos los datos auxiliares para los cálculos antes de seguir leyendo valores
        if (i == INTTIME) {
            writeOnInit();
        }
    }
}

int MainWindow::getAddressFromIndex(int index) {
    // Direccion correspondiente al campo elegido
    switch (index) {
    case ZERO: return INT_TIME_ADDRESS;
    case ONE: return INT_PERIOD_ADDRESS;
    case TWO: return GPOL_ADDRESS;
    default: return customVariable.toInt();
    }
}

bool MainWindow::validateValueByType(int index, uint32_t value) {
    // Comprobar los valores
    if ((index == INTTIME || index == INTPERIOD) && value < ONE) {
        QMessageBox::warning(this, "Error", "Value must be equal or greater than 1.");
        return false;
    }

    if (index == GPOL && (value < 1500 || value > 3600)) {
        QMessageBox::warning(this, "Error", "GPOL value must be between 1500 mV and 3600 mV.");
        return false;
    }

    return true;
}

void MainWindow::updateReadOnlyField(int index, uint32_t value) {
    if (index == INTTIME || index == INTPERIOD) {
        readOnlyFields[index]->setText(QString::number(value));
    }
    // Hay que compensar los indices a partir del period, ya que esta el indice de Tframe entre medias
    else if (index == GPOL) {
        readOnlyFields[++index]->setText(QString::number(value));
    } else {
        readOnlyFields[++index]->setText(QString("0x%1").arg(value, 8, 16, QLatin1Char('0')).toUpper().replace("X", "x"));
    }
}

void MainWindow::readMCKRegister() {
    // Primero leemos el registro de los outputs, para tener los valores de los cálculos posteriores
    readOutputRegister();

    int reg = stoi(MCKREGISTER, nullptr, 16);
    vector<uint8_t> response;

    if (!sendReadRequest(reg, response)) return;
    if (!sanitizeResponse(response)) return;

    uint32_t value = 0;
    if (!validateAndExtractValue(response, value)) return;

    // Necesitamos los 2 primeros bytes
    value = value & 0xFFFF;

    uint8_t clkSRCbits = value & 0x01; // Primer bit
    uint8_t mckDIVbits = (value >> 4) & 0x03; // Bits 4 y 5
    uint8_t xclkDIV = (value >> 8) & 0xFF; // Segundo byte entero

    int clkSRC = decodeClkSource(clkSRCbits); // Calcular el clkSRC
    int mckDIV = decodeMckDiv(mckDIVbits); // Calcular el mckDIV

    calculateMCKValues(clkSRC, xclkDIV, mckDIV); // Calcular los valores del MCK con todos los datos anteriores
}

int MainWindow::decodeClkSource(uint8_t clkSRCbits) {
    // Decodificamos los bits del CLK
    switch (clkSRCbits) {
    case ZERO: return CLK;
    case ONE:
        QMessageBox::warning(this, "Warning", "External clock source selected");
        return CLK; // Placeholder, no vendrá así configurado pero hay que contemplarlo
    default: return CLK;
    }
}

int MainWindow::decodeMckDiv(uint8_t mckDIVbits) {
    // Decodificamos los bits del MCK
    switch (mckDIVbits) {
    case ZERO: return FIRST; // 00
    case ONE: return SECOND; // 01
    case TWO: return THIRD; // 10
    default: return FIRST;
    }
}

void MainWindow::calculateMCKValues(int clkSRC, int xclkDIV, int mckDIV) {
    // Hacemos los cálculos del MCK una vez tenemos el resto de datos de los bits
    float XClk = (clkSRC * E6) * OPERATION / xclkDIV;
    MCK = XClk / mckDIV;

    try {
        // Operaciones sacadas del manual, hay que hacer cambios de unidades en alguna
        MinTFrame = (readOnlyFields[INTTIME]->text().toUInt() / MCK) + (pixels / (MCK * outputs));
        FPS = ONE / MinTFrame;

        fpsBox->setText(QString::number(FPS));
        tframeBox->setText(QString::number(MinTFrame));
        radioITR->setChecked(true);
    } catch (...) {
        // Si no salen los cálculos (nunca debería pasar pero es una posibilidad teórica) cambiamos de modo
        // Si esto pasa la cámara se descalibra y la imagen pierde frames y salen píxeles erróneos
        // Evitamos esto gestionando el PERIOD de forma manual sin que el usuario lo manipule, por el momento

        QMessageBox::warning(this, "Error", "Invalid MCK for ITR Mode");
        radioIWR->setChecked(true);
    }
}

void MainWindow::readOutputRegister() {
    int reg = stoi(OUTPUTREGISTER, nullptr, 16);

    vector<uint8_t> response;
    if (!sendReadRequest(reg, response)) return;
    if (!sanitizeResponse(response)) return;

    uint32_t value = 0;
    if (!validateAndExtractValue(response, value)) return;

    value = value & 0xFFFF; // 2 primeros bytes

    pixels = decodeResolution((value >> 6) & 0x03); // Bytes 6 y 7
    outputs = ((value >> 5) & 0x01) ? FOUR_VIDEO_OUTPUTS : TWO_VIDEO_OUTPUTS; // Bit 5

    // Forzar a que sean 4 a modo de placeholder para evitar fallos
    // Debería tener 4 en el registro sin embargo salen 2, a pesar que la imagen se muestra para 4 con datos de 4...
    outputs = FOUR_VIDEO_OUTPUTS;
}

int MainWindow::decodeResolution(uint8_t resBits) {
    // Calculamos resolución máxima
    switch (resBits) {
    case ZERO: return FIRSTRES; // 00
    case ONE: return SECONDRES; // 01
    case TWO: return THIRDRES; // 10
    default: {
            // Random, no debería ocurrir nunca este caso si está correctamente configurada de fábrica
            int options[] = { FIRSTRES, SECONDRES, THIRDRES };
            return options[QRandomGenerator::global()->bounded(3)];
        }
    }
}

bool MainWindow::validateCRC(const vector<uint8_t>& response) {
    // Solo pasar la respuesta completa al manejador, que se encargará de la validación.
    bool isValid = serialManager->validateCRC(response);  // El manejador calcula y valida el CRC.

    // Si el CRC es válido, imprimimos el CRC calculado y el recibido.
    if (isValid) {
        // Extraemos el CRC recibido (últimos dos bytes de la respuesta).
        uint16_t receivedCRC = (response[response.size() - 2] << 8) | response[response.size() - 1];  // Big-endian CRC

        qDebug() << "CRC Calculated and received are both valid: 0x" << QString::number(receivedCRC, 16).toUpper();
    } else {
        qDebug() << "CRC not valid.";
    }

    return isValid;  // Devuelve si el CRC es válido (true o false)
}

void MainWindow::bitDecode(uint32_t data) {
    // Para poder hacer un debug más completo bit a bit para ciertos registros, mostramos los datos en binario
    bitset<32> binaryData = data;
    string segmentedData = "", aux = "";

    for (int i = 0; i < 8; i++) {
        aux = binaryData.to_string().substr(i * 4, 4); // Cada bit de 4 en 4 se añade con un espacio
        segmentedData.append(aux).append(" ");
        // "0000 0000 0000 0010 1010 1110 1011 1000" -> 175800 / 0x2AEB8
    }

    qDebug() << "Complete data in bits: " << segmentedData;
}

// Agregar la nueva función para manejar la selección del puerto
void MainWindow::handlePortSelection(int index) {
    // Si no hay puerto seleccionado (index == -1), deshabilitamos el botón de desconectar
    if (index == -1) {
        disconnectPortButton->setEnabled(false);
    } else {
        // Si se selecciona un puerto, habilitamos el botón de desconectar
        disconnectPortButton->setEnabled(true);
    }
}

void MainWindow::disconnectSerialPort() {
    if (selectedPort.isEmpty()) return;  // No hacer nada si no hay puerto seleccionado

    serialManager->closePort();  // Cierra el puerto

    // Mostrar la notificación solo una vez
    QMessageBox::information(this, "Disconnected", QString("Disconnected from port: %1").arg(selectedPort));

    selectedPort.clear();  // Borra la selección del puerto
    portSelector->setCurrentIndex(-1);  // Restablece el dropdown a su estado inicial
    setControlsEnabled(false);  // Deshabilita los controles
    disconnectPortButton->setEnabled(false);  // Deshabilita el botón de desconectar
    clearFields(); // Limpia el texto de los campos
}

void MainWindow::clearFields() {
    customVariable.clear();

    // Recorremos todos los campos y los limpiamos
    for (int index = 0; index <= VARIABLES_QUANTITY; ++index) {
        if (index == INTTIME || index == INTPERIOD) {
            readOnlyFields[index]->clear();
        }
        // Hay que compensar los indices a partir del period, ya que esta el indice de Tframe entre medias
        else {
            readOnlyFields[++index]->setText("");
        }
    }
    readOnlyFields[VARIABLES_QUANTITY+1]->clear(); // Acceder al valor del registro custom manualmente para poder limpiarlo
    fpsBox->clear(); // Acceder al campo de fps para limpiarlo manualmente
    tframeBox->clear(); // Acceder al campo de tframe para limpiarlo manualmente

    // Tambien los campos de escritura de input del usuario
    for (auto &field : writeFields) {
        field->clear();
    }

}

void MainWindow::monitorForcedDisconnects() {
    // Si no estamos conectados no hace nada
    if (portSelector->currentIndex() == -1) return;

    // Obtenemos el puerto actual
    QString currentPortName = selectedPort;

    // Refrescamos la lista de puertos en una nueva lista y vemos si sigue disponible el que tenemos conectado
    const auto ports = QSerialPortInfo::availablePorts();
    bool portStillAvailable = false;
    for (const QSerialPortInfo &info : ports) {
        if (info.portName() == currentPortName) {
            portStillAvailable = true;
            break;
        }
    }

    // Si no aparece porque se ha desconectado, salimos
    // Windows no siempre detecta que el puerto serial deja de estar disponible, sobretodo si se desconecta el cable de forma súbita
    // Con esta monitorización extra cubrimos los casos cuando falla porque sigue apareciendo como disponible
    if (!portStillAvailable || !serialManager->checkPort()) {
        // Cogemos el indice del puerto que ya no esta disponible y lo borramos, luego desconectamos
        int aux = portSelector->currentIndex();
        disconnectSerialPort();
        portSelector->removeItem(aux);
    }
}
