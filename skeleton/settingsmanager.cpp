#include "settingsmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "../etc/consts.h"
#include "loggerstream.h"

const Language SettingsManager::defaultLanguage = Language::ENGLISH;
const QString SettingsManager::defaultLanguageStr = "en_US";
const QString SettingsManager::defaultTheme = "singularity";
const int SettingsManager::defaultSlideAnim = 200;

const QStringList SettingsManager::changableSettings = {
    "Language",
    "Theme",
    "SlideAnimation"
};

const QJsonObject SettingsManager::defaultSettings = {
    {"Language", defaultLanguageStr},
    {"FirstRun", true},
    {"Version", APP_VERSION},
    {"Theme", defaultTheme},
    {"SlideAnimation", defaultSlideAnim}
};

static bool isInteger(const QJsonValue& value) {
  if (!value.isDouble()) return false;

  double d = value.toDouble();
  return d == static_cast<int>(d) &&
         d >= INT_MIN && d <= INT_MAX;
}

bool SettingsManager::validateAndFixSettings(QJsonObject& settingsObj) {
  bool needsUpdate = false;
  needsUpdate |= controlKeys(settingsObj);
  needsUpdate |= checkversion(settingsObj);
  needsUpdate |= checklang(settingsObj);
  needsUpdate |= checktheme(settingsObj);
  needsUpdate |= checkslideanim(settingsObj);

  sinfo << "Settings validated. NEEDS UPDATE : " << (needsUpdate ? "YES" : "NO");
  return needsUpdate;
}

// helpers
bool SettingsManager::controlKeys(QJsonObject& settingsObj) {
  bool needsUpdate = false;

  // delete undefined keys
  for (auto it = settingsObj.begin(); it != settingsObj.end();) {
    if (!defaultSettings.contains(it.key())) {
      swrn << "Undefined key found: " + it.key() + ", deleting it.";
      it = settingsObj.erase(it);
      needsUpdate = true;
    } else {
      ++it;
    }
  }

  // add missing keys
  for (auto it = defaultSettings.begin(); it != defaultSettings.end(); ++it) {
    if (!settingsObj.contains(it.key())) {
      settingsObj[it.key()] = it.value();
      swrn << "The key " + it.key() + " cannot be found, adding it";
      needsUpdate = true;
    }
  }

  return needsUpdate;
}

bool SettingsManager::checkversion(QJsonObject& settingsObj) {
  bool needsUpdate = false;
  if (settingsObj.contains("Version") && settingsObj["Version"].isString()) {
    QString verinfile = settingsObj["Version"].toString();
    if (verinfile != APP_VERSION) {
      settingsObj["Version"] = APP_VERSION;
      swrn << "Updating settings file version from " + verinfile + " to " +
                  APP_VERSION;
      needsUpdate = true;
    }
  } else {
    settingsObj["Version"] = APP_VERSION;
    swrn << "Version not found or invalid, added";
    needsUpdate = true;
  }
  return needsUpdate;
}

bool SettingsManager::checklang(QJsonObject& settingsObj) {
  bool needsUpdate = false;

  if (settingsObj.contains("Language") && settingsObj["Language"].isString()) {
    QString langinfile = settingsObj["Language"].toString(defaultLanguageStr);
    if (!supportedLanguages.contains(langinfile)) {
      settingsObj["Language"] = defaultLanguageStr;
      needsUpdate = true;
      swrn << QString("Undefined language found (%1), defaulted it to %2").arg(langinfile, defaultLanguageStr);
    }
  } else {
    settingsObj["Language"] = defaultLanguageStr;
    swrn << "Language not found or invalid, added";
    needsUpdate = true;
  }
  return needsUpdate;
}

bool SettingsManager::checktheme(QJsonObject& settingsObj) {
  bool needsUpdate = false;

  if (settingsObj.contains("Theme") && settingsObj["Theme"].isString()) {
    QString themeinfile = settingsObj["Theme"].toString(defaultTheme);
    if (!supportedThemes.contains(themeinfile)) {
      settingsObj["Theme"] = defaultTheme;
      needsUpdate = true;
      swrn << QString("Undefined theme found (%1), defaulted it to %2").arg(themeinfile, defaultTheme);
    }
  } else {
    settingsObj["Theme"] = defaultTheme;
    swrn << "Theme key not found or invalid, added";
    needsUpdate = true;
  }

  return needsUpdate;
}

bool SettingsManager::checkslideanim(QJsonObject& settingsObj) {
  bool needsUpdate = false;

  if (settingsObj.contains("SlideAnimation") && isInteger(settingsObj["SlideAnimation"])) {
    int animinfile = settingsObj["SlideAnimation"].toInt(defaultSlideAnim);
    if (animinfile < 50 || animinfile > 2000) {
      settingsObj["SlideAnimation"] = defaultSlideAnim;
      needsUpdate = true;
      swrn << QString("Invalid slide animation int found (%1), defaulted it to %2").arg(animinfile, defaultSlideAnim);
    }
  } else {
    settingsObj["SlideAnimation"] = defaultSlideAnim;
    swrn << "SlideAnimation key not found or invalid, added";
    needsUpdate = true;
  }

  return needsUpdate;
}

QString SettingsManager::getCurrencyKey() {
  QSettings settings("../config.ini", QSettings::IniFormat);
  return settings.value("API/CURRENCY_KEY").toString();
}
