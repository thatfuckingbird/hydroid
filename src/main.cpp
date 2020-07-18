/*
Hydroid
Copyright (C) 2020  Hydroid developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <cstdlib>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QIcon>
#include <QQuickStyle>
#include <QtSvg>
#include "thumbgridmodel.h"
#include "taglistmodel.h"
#include "hydrusapi.h"
#include "hydroidsettings.h"
#include "hydroidnativeutils.h"
#include "hydrusapi.h"
#include "hydroidimageprovider.h"
#include "version.h"
#include "metadatacache.h"
#include "filecache.h"

#ifdef Q_OS_ANDROID
#include <QtAndroidExtras/QtAndroid>
#endif

void debugMsgHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char* file = context.file ? context.file : "";
    const char* function = context.function ? context.function : "";
    switch(type)
    {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
            break;
    }
}

QString getVersionString()
{
    QString templateStr{"Hydroid %1.%2.%3-%4"};
    if(QString{HYDROID_VERSION_LABEL}.isEmpty()) templateStr = QString{"Hydroid %1.%2.%3%4"};
    return templateStr.arg(QString::number(HYDROID_VERSION_MAJOR), QString::number(HYDROID_VERSION_MINOR), QString::number(HYDROID_VERSION_PATCH), QString{HYDROID_VERSION_LABEL});
}

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    qmlRegisterType<ThumbGridModel>("Hydroid", 1, 0, "ThumbGridModel");
    qmlRegisterType<TagListModel>("Hydroid", 1, 0, "TagListModel");

    QQuickStyle::setStyle("Material");

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/assets/icons/icon.png"));
    app.setApplicationDisplayName("Hydroid");
    app.setApplicationName("Hydroid");
    app.setOrganizationName("Hydroid");
    app.setOrganizationDomain({});
    app.setQuitOnLastWindowClosed(true);

#ifdef Q_OS_ANDROID
    QtAndroid::requestPermissions({"android.permission.INTERNET"}, [](const QtAndroid::PermissionResultMap&){});
#endif

    //qInstallMessageHandler(&debugMsgHandler);

    QCommandLineParser* parser = new QCommandLineParser{};
    parser->setApplicationDescription("Hydroid");
    parser->addOption({QStringList() << "help"
                                     << "h",
                       "Help"});
    parser->addOption({QStringList() << "license"
                                     << "l",
                       "License information"});
    parser->addOption({QStringList() << "version"
                                     << "v",
                       "Version"});
    parser->process(app);

    if(parser->isSet("help"))
    {
        QTextStream out(stdout);
        out << parser->helpText();
        return EXIT_SUCCESS;
    }
    else if(parser->isSet("license"))
    {
        QTextStream out(stdout);
        out << QString{HYDROID_LICENSE_TEMPLATE}.arg(getVersionString(), QString{HYDROID_COPYRIGHT_YEARS});
        return EXIT_SUCCESS;
    }
    else if(parser->isSet("version"))
    {
        QTextStream out(stdout);
        out << getVersionString() << Qt::endl;
        return EXIT_SUCCESS;
    }

    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/src/main.qml"));
    QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject* obj, const QUrl& objUrl) {
          if(!obj && url == objUrl) QCoreApplication::exit(-1);
      },
      Qt::QueuedConnection);

    engine.rootContext()->setContextProperty("hydroidSettings", &HydroidSettings::hydroidSettings());
    auto loadEngine = [&]() {
        engine.addImageProvider("hyimg", new HydroidImageProvider{});
        engine.rootContext()->setContextProperty("hydroidNativeUtils", new HydroidNativeUtils{});
        engine.rootContext()->setContextProperty("hydrusAPI", &HydrusAPI::hydrusAPI());
        engine.rootContext()->setContextProperty("metadataCache", &MetadataCache::metadataCache());
        engine.rootContext()->setContextProperty("fileCache", &FileCache::fileCache());
        engine.rootContext()->setContextProperty("HYDROID_VERSION_MAJOR", HYDROID_VERSION_MAJOR);
        engine.rootContext()->setContextProperty("HYDROID_VERSION_MINOR", HYDROID_VERSION_MINOR);
        engine.rootContext()->setContextProperty("HYDROID_VERSION_PATCH", HYDROID_VERSION_PATCH);
        engine.rootContext()->setContextProperty("HYDROID_VERSION_LABEL", HYDROID_VERSION_LABEL);
        engine.rootContext()->setContextProperty("HYDROID_EDITION_TEXT", HYDROID_EDITION_TEXT);
        engine.rootContext()->setContextProperty("HYDROID_COPYRIGHT_YEARS", HYDROID_COPYRIGHT_YEARS);

        engine.load(url);
    };

#ifdef Q_OS_WASM
    if(HydroidSettings::hydroidSettings().status() == QSettings::NoError) {
        loadEngine();
    } else {
        QObject::connect(&HydroidSettings::hydroidSettings(), &HydroidSettings::settingsReady, loadEngine);
    }
#else
    loadEngine();
#endif

    return app.exec();
}
