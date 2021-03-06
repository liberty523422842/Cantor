/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */

#include <QApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QTextStream>

#include "juliaserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "Can't connect to the D-Bus session bus.\n"
                      "To start it, run: eval `dbus-launch --auto-syntax`";
        return 1;
    }

    const QString &serviceName =
        QString::fromLatin1("org.kde.Cantor.Julia-%1").arg(app.applicationPid());

    if (!QDBusConnection::sessionBus().registerService(serviceName)) {
        qWarning() << QDBusConnection::sessionBus().lastError().message();
        return 2;
    }

    JuliaServer server;
    QDBusConnection::sessionBus().registerObject(
        QLatin1String("/"),
        &server,
        QDBusConnection::ExportAllSlots
    );

    QTextStream(stdout) << "ready" << endl;

    return app.exec();
}
