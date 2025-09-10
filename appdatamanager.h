#ifndef APPDATAMANAGER_H
#define APPDATAMANAGER_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QStandardPaths>
/*
 * singleton yapısı, normal classlardan farkı bu, program boyunca çalışır
 * ve aşağıdaki ayarlar sayesinde kopyalanamaz. Sadece 1 tane vardır.
 * Erişimi kolaydır, AppDataManager::instace() ile tüm slotlara sinyallere
 * ulaşılabilir. Ayrıca sinyal/slot yapısı bunlarda daha kolaydır
 */
class AppDataManager : public QObject {
  Q_OBJECT
 public:
  static AppDataManager& instance() {
    static AppDataManager _instance;
    return _instance;
  }

  // fonksiyonlar
  bool ensureAppDataFolderExists();
  QString appFolderPath();

  // settings.json için
  bool checkSettingsFileExists();
  QString settingsFilePath();
  bool createSettingsFile();
  bool fixSettingsFile();

  // assets
  QString assetUrl(const QString& relativePath);
  bool ensureDefaultAssets();
  QString getPatchNotes();

  bool saveJson(const QString& filepath, const QJsonObject& obj);

 private:
  explicit AppDataManager(QObject* parent = nullptr) : QObject(parent) {}

  QString m_patchnotesmd;
  bool downloadFile(const QString& relpath, const QString& customLocalPath = "");

  // Copy ve assign engelle
  AppDataManager(const AppDataManager&) = delete;
  AppDataManager& operator=(const AppDataManager&) = delete;

  ~AppDataManager() override = default;
};

#endif  // APPDATAMANAGER_H
