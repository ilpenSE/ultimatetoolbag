#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "logger.h"
#include "languagemanager.h"
#include "thememanager.h"
#include "appdatamanager.h"
#include "settingsmanager.h"
#include "versionmanager.h"
#include "consts.h"
#include "instances.h"
#include "LoggerStream.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("UltimateToolbag");
  app.setOrganizationName("ilpeNdev");

  // logger and version manager init
  VersionManager::instance().initVer(APP_VERSION);
  Logger::instance().logInfo("Logger initialized");

  // AppData manager init
  if (!_appdataman().ensureAppDataFolderExists()) {
    fserr() << "Ensurement of appdata folder failed!";
    return -1;
  }

  // settings init

  // language init

  // theme init

  MainWindow w;
  w.show();
  return app.exec();
}
