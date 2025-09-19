#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>

#include "skeleton/loggerstream.h"
#include "skeleton/fileorganizer.h"
#include "etc/consts.h"
#include "etc/instances.h"
#include "skeleton/languagemanager.h"
#include "skeleton/logger.h"
#include "forms/mainwindow.h"
#include "skeleton/settingsmanager.h"
#include "skeleton/thememanager.h"
#include "skeleton/versionmanager.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("UltimateToolbag");
  app.setOrganizationName("ilpeNdev");

  // logger and version manager init
  _verman.initVer(APP_VERSION);
  Logger::instance().logInfo("Logger initialized");

  // AppData manager init
  if (!_forg.ensureAppDataFolderExists()) {
    fscrit << "Ensurement of appdata folder failed!";
    QMessageBox::critical(
        nullptr, "Fatal Error",
        "Ensurement of appdata folder failed!");
    return -1;
  }

  bool appUpdateAvailable = false;

  if (!_DEBUG_MODE) {
    // update checker
    appUpdateAvailable = _forg.updateAvailable();
    if (appUpdateAvailable) {
      linfo << "New version of app available!";
    } else {
      linfo << "No new version available";
    }

    if (!_forg.ensureDefaultAssets()) {
      fscrit << "Ensurement of default assets failed!";
      QMessageBox::critical(
          nullptr, "Fatal Error",
          "Default assets couldn't be downloaded. Check your internet connection.");
      return -1;
    }
  }

  // groups.json init
  QString errorM;
  if (!_groupman.ensureJsonFile(&errorM)) {
    fscrit << "Failed to ensure groups.json: " << errorM;
    return -1;
  }

  // settings init
  QString settingsPath = _forg.settingsFilePath();

  bool ok = false;
  QJsonObject settings = _settingsman.loadSettings(settingsPath, ok);

  if (!ok) {
    // set defaults when settings file corrupted or missing
    settings = _settingsman.defaultSettings();

    // use system default language on first run
    settings["Language"] = _langman.getsyslang();
    settings["FirstRun"] = false;

    _forg.saveJson(settingsPath, settings);
    fswrn << "Settings file was deleted or corrupted, created one.";
  } else {
    if (_settingsman.validateAndFixSettings(settings)) {
      if (!_forg.saveJson(settingsPath, settings)) {
        fscrit << "Settings file saving failed after validation";
        QMessageBox::critical(nullptr, "Fatal Error", "Settings file saving failed!");
        return -1;
      }
    }
  }

  // language init
  QString savedLanguage = settings["Language"].toString(SettingsManager::defaultLanguageStr);
  if (settings["FirstRun"].toBool()) savedLanguage = LanguageManager::getsyslang();
  lninfo << "Loading language from settings: " << savedLanguage;
  if (!_langman.loadLanguage(savedLanguage)) {
    lnerr << "Language couldn't be loaded: " << savedLanguage << ", retrying as english...";
    // Fallback, try as english
    if (!_langman.loadLanguage(SettingsManager::defaultLanguageStr)) {
      lncrit << "Language initialization failed!";
      QMessageBox::critical(nullptr, "Fatal Error", "Language couldn't be loaded.");
      return -1;
    }
  }

  // themes init
  QString themesPath = _forg.appFolderPath() + "/themes.json";

  bool themesOk = false;
  QJsonObject themes = _settingsman.loadSettings(themesPath, themesOk);

  if (!themesOk) {
    // create default themes.json
    QJsonObject defaults;
    defaults["dark"] = "Fluent Dark";
    defaults["light"] = "Fluent Light";

    if (!_forg.saveJson(themesPath, defaults)) {
      fscrit << "Themes.json saving failed while creating";
      QMessageBox::critical(nullptr, "Fatal Error", "Themes.json saving failed while creating");
      return -1;
    }
    fswrn << "Themes file was deleted or corrupted, created one.";
    themes = defaults;
  } else {
    // add missing keys
    bool needsUpdate = false;
    if (!themes.contains("dark")) {
      themes["dark"] = "Fluent Dark";
      needsUpdate = true;
    }
    if (!themes.contains("light")) {
      themes["light"] = "Fluent Light";
      needsUpdate = true;
    }

    if (needsUpdate) {
      if (!_forg.saveJson(themesPath, themes)) {
        fscrit << "Themes.json saving failed after updating";
        QMessageBox::critical(nullptr, "Fatal Error", "Themes.json saving failed after updating");
        return -1;
      }
    }
  }

  // load the theme
  QString theme = settings["Theme"].toString(_settingsman.defaultTheme);
  if (!_themesman.applyTheme(_themesman.getVisibleName(theme))) {
    thcrit << "Theme couldn't be loaded, file name: " << theme
            << " visible name: "
            << _themesman.getVisibleName(theme);
    QMessageBox::critical(nullptr, "Fatal Error", "Theme couldn't be loaded.");
    return -1;
  }

  // load favs.json
  if (!_favman.ensureFavs()) {
    fscrit << "Favorites JSON file couldn't be loaded!";
    QMessageBox::critical(nullptr, "Fatal Error", "Favorites couldn't be loaded.");
    return -1;
  }

  // use this on file saving instead of destructor
  QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
                   [](){
                     _groupman.saveOnExit();
                     _favman.saveOnExit();
                   });

  MainWindow w(settings, appUpdateAvailable);
  w.show();
  return app.exec();
}
