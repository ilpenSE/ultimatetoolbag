#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QRegularExpression>

#include "consts.h"
#include "languagemanager.h"
#include "versionmanager.h"

class SettingsManager : public QObject {
  Q_OBJECT
 public:
  static SettingsManager& instance() {
    static SettingsManager _instance;
    return _instance;
  }

  // default ayarlar
  QJsonObject defaultSettings() const {
    QJsonObject obj;
    obj["Language"] = "en_US";
    obj["DefaultHotkey"] = "F6";  // modifier destekli
    obj["FirstRun"] = true;
    obj["ActiveMacro"] = 1;  // sqlite içindeki id
    obj["Version"] = APP_VERSION;
    obj["Theme"] = "dark";
    return obj;
  }

  // fonksiyonlar
  void validateAndFixSettings(QJsonObject& settingsObj);
  bool saveSettings(const QString& path, const QJsonObject& settingsObj);
  QJsonObject loadSettings(const QString& path, bool& ok);

 private:
  const QStringList supportedLanguages =
      LanguageManager::instance().localeToLanguageMap.keys();
  const QString currentVersion = VersionManager::instance().version();

  explicit SettingsManager(QObject* parent = nullptr) : QObject(parent) {}

  // Copy ve assign engelle
  SettingsManager(const SettingsManager&) = delete;
  SettingsManager& operator=(const SettingsManager&) = delete;

  ~SettingsManager() override = default;
};

#endif  // SETTINGSMANAGER_H
