#pragma once

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QStandardPaths>
#include <QJsonObject>

/*
 * meyers singleton yapısı, normal classlardan farkı bu, program boyunca çalışır
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

  bool ensureThemesJson(const QString& filepath, QString* erro = nullptr);

  // assets
  QString assetUrl(const QString& relativePath);
  bool ensureDefaultAssets();
  QString getPatchNotes();
  bool assetExists(const QString& relpath);
  QFile getasset(const QString& relpath);

  // relpath like this: themes/dark.qss
  bool downloadAsset(const QString& relpath);

  void checkFolder(const QString& folder, QDir dir);

 private:
  explicit FileOrganizer(QObject* parent = nullptr) : QObject(parent) {
    defaultThemesObj = {
        {"dark", "Fluent Dark"},
        {"light", "Fluent Light"},
        {"synthwave", "Synthwave"},
        {"singularity", "Singularity"}
    };
  }

  QString m_patchnotesmd;
  QJsonObject defaultThemesObj;

  // block copy and assign
  FileOrganizer(const FileOrganizer&) = delete;
  FileOrganizer& operator=(const FileOrganizer&) = delete;

  ~FileOrganizer() override = default;
};
