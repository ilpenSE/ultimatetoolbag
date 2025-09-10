#include "appdatamanager.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include "consts.h"
#include "logger.h"
#include "settingsmanager.h"
#include "LoggerStream.h"
#include "etagcontroller.h"
#include "instances.h"

QString AppDataManager::appFolderPath() {
  QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

  if (base.isEmpty()) {
    // Fallback
    base = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  }

  if (base.isEmpty()) {
    base = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    return QDir(base).filePath(".UltimateToolbag");
  }

  QDir().mkpath(base);

  return base;
}

bool AppDataManager::ensureAppDataFolderExists() {
  QDir dir(appFolderPath());
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      fserr() << "Directory cannot be created: "  << appFolderPath();
      return false;
    }
    fsinfo() << "Directory created: " << appFolderPath();
  } else {
    fsinfo() << "Directory already exists: " << appFolderPath();
  }

  // logs klasörü
  QString logsPath = dir.filePath("logs");
  QDir logsDir(logsPath);
  if (!logsDir.exists()) {
    if (logsDir.mkpath(".")) {
      fsinfo() << "Logs folder created: " + logsPath;
    } else {
      fserr() << "Logs folder cannot be created: " << logsPath;
    }
  }

  // cache klasörü
  QString cachePath = dir.filePath("cache");
  QDir cacheDir(cachePath);
  if (!cacheDir.exists()) {
    if (cacheDir.mkpath(".")) {
      fsinfo() << "Cache folder created: " + cachePath;
    } else {
      fserr() << "Cache folder cannot be created: " + cachePath;
    }
  }

  return true;
}

QString AppDataManager::settingsFilePath() {
  return QDir(appFolderPath()).filePath("settings.json");
}

bool AppDataManager::checkSettingsFileExists() {
  return QFile::exists(settingsFilePath());
}

bool AppDataManager::createSettingsFile() {
  if (checkSettingsFileExists()) return true;
  QJsonObject defaultSettings = SettingsManager::instance().defaultSettings();

  fsinfo() << "Creating settings.json file";
  return saveJson(settingsFilePath(), defaultSettings);
}

bool AppDataManager::fixSettingsFile() {
  QFile file(settingsFilePath());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fswrn() << "Settings.json cannot be opened, will be created.";
    return createSettingsFile();
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    fswrn() << "Settings.json parse error, will be created.";
    return createSettingsFile();
  }

  QJsonObject settingsObj = doc.object();
  SettingsManager::instance().validateAndFixSettings(settingsObj);

  return saveJson(settingsFilePath(), settingsObj);
}

QString AppDataManager::assetUrl(const QString& relativePath) {
  return QString("https://raw.githubusercontent.com/%1/%2/%3/%4")
      .arg(GITHUB_USER)
      .arg(GITHUB_REPO)
      .arg(EXTERNAL_ASSETS_BRANCH)
      .arg(relativePath);
}

bool AppDataManager::downloadFile(const QString& relpath, const QString& customLocalPath) {
  QString downloadUrl = assetUrl(relpath);

  QString localPath = QDir(appFolderPath()).filePath(customLocalPath.isEmpty() ? relpath : customLocalPath);

  QNetworkAccessManager mgr;
  QNetworkRequest req{ QUrl(downloadUrl) };
  QNetworkReply* reply = mgr.get(req);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    fserr() << "Cannot download " << relpath << ": " << reply->errorString();
    reply->deleteLater();
    return false;  // internet yok veya dosya bulunamadı
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  QDir().mkpath(QFileInfo(localPath).absolutePath());
  QFile f(localPath);
  if (f.open(QIODevice::WriteOnly)) {
    f.write(data);
    f.close();
    fsinfo() << "Downloaded and saved: " + relpath;
    return true;
  } else {
    fserr() << "Cannot write file: " + localPath;
    return false;
  }
}

QString AppDataManager::getPatchNotes() {
  // Önce memory cache'e bak
  if (!m_patchnotesmd.isEmpty()) {
    return m_patchnotesmd;
  }

  // Sonra disk cache'e bak
  QString cachePath = QDir(appFolderPath()).filePath("patchnotes.md");
  if (QFile::exists(cachePath)) {
    QFile file(cachePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_patchnotesmd = file.readAll();
      return m_patchnotesmd;
    }
  }

  return "Cannot load patch notes.";
}

bool AppDataManager::ensureDefaultAssets() {
  // etag karşılaştırması
  QStringList d = _etagcon().compareEtags();

  if (d.isEmpty()) {
    fsinfo() << "All external assets are up to date";
    return true;
  }

  auto gitEtags = _etagcon().fetchEtags(_etagcon().getUrlMap());
  for (const QString& rpath : std::as_const(d)) {
    fsinfo() << "New version of " + rpath + " found, downloading.";
    if (downloadFile(rpath)) {
      // İndirme başarılı, ETag'i güncelle
      QString key = _etagcon().getKeyFromRelativePath(rpath);
      if (!key.isEmpty() && gitEtags.contains(key)) {
        _etagcon().updateEtagCache(key, gitEtags[key].toString());
      }
    }
  }
  return true;
}

bool AppDataManager::saveJson(const QString& filepath, const QJsonObject& obj) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    fswrn() << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(obj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}
