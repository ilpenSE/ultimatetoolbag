#ifndef FILEORGANIZER_H
#define FILEORGANIZER_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QStandardPaths>
/*
 * singleton yapısı, normal classlardan farkı bu, program boyunca çalışır
 * ve aşağıdaki ayarlar sayesinde kopyalanamaz. Sadece 1 tane vardır.
 * Erişimi kolaydır, FileOrganizer::instace() ile tüm slotlara sinyallere
 * ulaşılabilir. Ayrıca sinyal/slot yapısı bunlarda daha kolaydır
 */
class FileOrganizer : public QObject {
  Q_OBJECT
 public:
  static FileOrganizer& instance() {
    static FileOrganizer _instance;
    return _instance;
  }

  // fonksiyonlar
  bool ensureAppDataFolderExists();
  QString appFolderPath();
  QString vAssetPath(); // version-based assets
  bool updateAvailable();

  // settings.json için
  bool checkSettingsFileExists();
  QString settingsFilePath();
  bool createSettingsFile();
  bool fixSettingsFile();

  // assets
  QString assetUrl(const QString& relativePath);
  bool ensureDefaultAssets();
  QString getPatchNotes();
  bool assetExists(const QString& relpath);
  QFile getasset(const QString& relpath);

  QJsonDocument loadJsonDoc(const QString& filepath);
  QJsonObject loadJson(const QString& filepath, QString* errm = nullptr);
  QJsonArray loadJsonArr(const QString& filepath, QString* errm = nullptr);
  bool saveJson(const QString& filepath, const QJsonObject& obj);
  bool saveJson(const QString& filepath, const QJsonArray& obj);

  // relpath like this: themes/dark.qss
  bool downloadAsset(const QString& relpath);

 private:
  explicit FileOrganizer(QObject* parent = nullptr) : QObject(parent) {}

  QString m_patchnotesmd;

  // block copy and assign
  FileOrganizer(const FileOrganizer&) = delete;
  FileOrganizer& operator=(const FileOrganizer&) = delete;

  ~FileOrganizer() override = default;
};

#endif  // FILEORGANIZER_H
