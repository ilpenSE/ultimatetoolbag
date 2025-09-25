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

static bool isInteger(const QJsonValue& value) {
  if (!value.isDouble()) return false;

  double d = value.toDouble();
  return d == static_cast<int>(d) &&
         d >= INT_MIN && d <= INT_MAX;
}

bool SettingsManager::validateAndFixSettings(QJsonObject& settingsObj) {
  bool needsUpdate = false;
  QJsonObject defaults = defaultSettings();

  // delete undefined keys
  for (auto it = settingsObj.begin(); it != settingsObj.end();) {
    if (!defaults.contains(it.key())) {
      swrn << "Undefined key found: " + it.key() + ", deleting it.";
      it = settingsObj.erase(it);
      needsUpdate = true;
    } else {
      ++it;
    }
  }

  // add missing keys
  for (auto it = defaults.begin(); it != defaults.end(); ++it) {
    if (!settingsObj.contains(it.key())) {
      settingsObj[it.key()] = it.value();
      swrn << "The key " + it.key() + " cannot be found, adding it";
      needsUpdate = true;
    }
  }

  // version control
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

  // pre-defined defaults
  QString defLang = defaults["Language"].toString();
  QString defTheme = defaults["Theme"].toString();
  int defSlideAnim = defaults["SlideAnimation"].toInt();

  // language control
  if (settingsObj.contains("Language") && settingsObj["Language"].isString()) {
    QString langinfile = settingsObj["Language"].toString(defLang);
    if (!supportedLanguages.contains(langinfile)) {
      settingsObj["Language"] = defLang;
      needsUpdate = true;
      swrn << QString("Undefined language found (%1), defaulted it to %2").arg(langinfile, defLang);
    }
  } else {
    settingsObj["Language"] = defLang;
    swrn << "Language not found or invalid, added";
    needsUpdate = true;
  }

  // theme control
  if (settingsObj.contains("Theme") && settingsObj["Theme"].isString()) {
    QString themeinfile = settingsObj["Theme"].toString(defTheme);
    if (!supportedThemes.contains(themeinfile)) {
      settingsObj["Theme"] = defTheme;
      needsUpdate = true;
      swrn << QString("Undefined theme found (%1), defaulted it to %2").arg(themeinfile, defTheme);
    }
  } else {
    settingsObj["Theme"] = defTheme;
    swrn << "Theme key not found or invalid, added";
    needsUpdate = true;
  }

  // slide animation control (has to be between 50-2000)
  if (settingsObj.contains("SlideAnimation") && isInteger(settingsObj["SlideAnimation"])) {
    int animinfile = settingsObj["SlideAnimation"].toInt(defSlideAnim);
    if (animinfile < 50 || animinfile > 2000) {
      settingsObj["SlideAnimation"] = defSlideAnim;
      needsUpdate = true;
      swrn << QString("Invalid slide animation int found (%1), defaulted it to %2").arg(animinfile, defSlideAnim);
    }
  } else {
    settingsObj["SlideAnimation"] = defSlideAnim;
    swrn << "SlideAnimation key not found or invalid, added";
    needsUpdate = true;
  }

  sinfo << "Settings validated. NEEDS UPDATE : " << (needsUpdate ? "YES" : "NO");
  return needsUpdate;
}

QJsonObject SettingsManager::loadSettings(const QString& path, bool& ok) {
  ok = false;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fserr << "Settings file cannot be opened: " + path;
    return QJsonObject();
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    fserr << "Settings file parse error: " + parseError.errorString();
    return QJsonObject();
  }

  ok = true;
  return doc.object();
}
