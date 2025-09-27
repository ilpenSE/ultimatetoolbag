#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QRegularExpression>

#include "../etc/enums.h"
#include "../etc/consts.h"
#include "languagemanager.h"

class SettingsManager : public QObject {
  Q_OBJECT
 public:
  static SettingsManager& instance() {
    static SettingsManager _instance;
    return _instance;
  }

  // default ayarlar
  static const QJsonObject defaultSettings;

  // değiştirilebilir ayarlar
  static const QStringList changableSettings;

  static const Language defaultLanguage;
  static const QString defaultLanguageStr;
  static const QString defaultTheme;
  static const int defaultSlideAnim;

  // fonksiyonlar
  bool validateAndFixSettings(QJsonObject& settingsObj);
  bool controlKeys(QJsonObject& settingsObj);
  bool checkversion(QJsonObject& settingsObj);
  bool checklang(QJsonObject& settingsObj);
  bool checktheme(QJsonObject& settingsObj);
  bool checkslideanim(QJsonObject& settingsObj);

 private:
  const QStringList supportedLanguages = LanguageManager::localeToLanguageMap.keys();
  const QStringList supportedThemes = { "dark", "light", "singularity", "synthwave" };

  explicit SettingsManager(QObject* parent = nullptr) : QObject(parent) {}

  // Copy ve assign engelle
  SettingsManager(const SettingsManager&) = delete;
  SettingsManager& operator=(const SettingsManager&) = delete;

  ~SettingsManager() override = default;
};

#endif  // SETTINGSMANAGER_H
