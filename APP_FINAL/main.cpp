#include "mainwindow.h"

#include <QApplication>
#include <QIcon>
#include <QSystemTrayIcon>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("GTK"); // Estilo

    QIcon appIcon(":/icono.ico"); // Icono de Sensia (ventana)
    a.setWindowIcon(appIcon);

    QSystemTrayIcon trayIcon(appIcon); // Icono también en barra de tareas y todo
    trayIcon.setVisible(true);

    MainWindow w;
    w.show(); // Mostrar la ventana principal (la única en la app)
    return a.exec(); // Ejecutar la aplicación
}
