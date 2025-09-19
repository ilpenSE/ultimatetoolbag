#include "fileorganizer.h"

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>

#include "loggerstream.h"
#include "../etc/consts.h"
#include "../etc/instances.h"
#include "settingsmanager.h"
#include "assetvalidator.h"

QString FileOrganizer::appFolderPath() {
  QString base =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

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

QString FileOrganizer::vAssetPath() {
  return appFolderPath() + "/" + APP_VERSION;
}

bool FileOrganizer::ensureAppDataFolderExists() {
  QDir dir(appFolderPath());
  if (!dir.exists()) {
    if (!dir.mkpath(".")) {
      fserr << "Directory cannot be created: " << appFolderPath();
      return false;
    }
    fsinfo << "Directory created: " << appFolderPath();
  } else {
    fsinfo << "Directory already exists: " << appFolderPath();
  }

  // version asset folder
  QString vassetPath = dir.filePath(APP_VERSION);
  QDir vassetDir(vassetPath);
  if (!vassetDir.exists()) {
    if (vassetDir.mkpath(".")) {
      fsinfo << "Version asset folder created: " << vassetPath;
    } else {
      fserr << "Version asset folder cannot be created: " << vassetPath;
    }
  }

  // logs folder
  QString logsPath = dir.filePath("logs");
  QDir logsDir(logsPath);
  if (!logsDir.exists()) {
    if (logsDir.mkpath(".")) {
      fsinfo << "Logs folder created: " + logsPath;
    } else {
      fserr << "Logs folder cannot be created: " << logsPath;
    }
  }

  // cache folder
  QString cachePath = dir.filePath("cache");
  QDir cacheDir(cachePath);
  if (!cacheDir.exists()) {
    if (cacheDir.mkpath(".")) {
      fsinfo << "Cache folder created: " + cachePath;
    } else {
      fserr << "Cache folder cannot be created: " + cachePath;
    }
  }

  return true;
}

QString FileOrganizer::settingsFilePath() {
  return QDir(appFolderPath()).filePath("settings.json");
}

bool FileOrganizer::checkSettingsFileExists() {
  return QFile::exists(settingsFilePath());
}

bool FileOrganizer::createSettingsFile() {
  if (checkSettingsFileExists()) return true;
  QJsonObject defaultSettings = _settingsman.defaultSettings();

  fsinfo << "Creating settings.json file";
  return saveJson(settingsFilePath(), defaultSettings);
}

bool FileOrganizer::fixSettingsFile() {
  QFile file(settingsFilePath());
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fswrn << "Settings.json cannot be opened, will be created.";
    return createSettingsFile();
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    fswrn << "Settings.json parse error, will be created.";
    return createSettingsFile();
  }

  QJsonObject settingsObj = doc.object();
  SettingsManager::instance().validateAndFixSettings(settingsObj);

  return saveJson(settingsFilePath(), settingsObj);
}

QString FileOrganizer::assetUrl(const QString& relativePath) {
  return QString("https://raw.githubusercontent.com/%1/%2/%3/%4/%5")
      .arg(GITHUB_USER, GITHUB_REPO, EXTERNAL_ASSETS_BRANCH, APP_VERSION, relativePath);
}

bool FileOrganizer::downloadAsset(const QString& relpath) {
  QString downloadUrl = assetUrl(relpath);

  QNetworkAccessManager mgr;
  QNetworkRequest req{QUrl(downloadUrl)};
  QNetworkReply* reply = mgr.get(req);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    fserr << "Cannot download " << relpath << ": " << reply->errorString();
    reply->deleteLater();
    return false;
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  QString localPath = QDir(vAssetPath()).filePath(relpath);

  QDir().mkpath(QFileInfo(localPath).absolutePath());
  QFile f(localPath);
  if (f.open(QIODevice::WriteOnly)) {
    f.write(data);
    f.close();
    fsinfo << "Downloaded and saved: " + relpath;
    return true;
  } else {
    fserr << "Cannot write file: " + localPath;
    return false;
  }
}

QFile FileOrganizer::getasset(const QString& relpath) {
  return QFile(vAssetPath() + "/" + relpath);
}

QString FileOrganizer::getPatchNotes() {
  // checkout memory cache
  if (!m_patchnotesmd.isEmpty()) {
    return m_patchnotesmd;
  }

  // checkout the file system caches
  QString cachePath = QDir(vAssetPath()).filePath("patchnotes.md");
  if (QFile::exists(cachePath)) {
    QFile file(cachePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      m_patchnotesmd = file.readAll();
      return m_patchnotesmd;
    }
  }

  return "Cannot load patch notes.";
}

QJsonDocument FileOrganizer::loadJsonDoc(const QString& filepath) {
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fserr << "Json file cannot be opened: " + filepath;
    return {};
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    fserr << "Json file parse error: " + parseError.errorString();
    return {};
  }
  return doc;
}

QJsonObject FileOrganizer::loadJson(const QString& filepath, QString* error) {
  QJsonDocument d = loadJsonDoc(filepath);
  if (!d.isObject()) if (error) *error = filepath + " is not an JSON object!";
  return loadJsonDoc(filepath).object();
}

QJsonArray FileOrganizer::loadJsonArr(const QString& filepath, QString* error) {
  QJsonDocument d = loadJsonDoc(filepath);
  if (!d.isArray()) { if (error) *error = filepath + " is not an JSON array!"; }
  return loadJsonDoc(filepath).array();
}

bool FileOrganizer::assetExists(const QString& relpath) {
  return QFile::exists(vAssetPath() + "/" + relpath);
}

bool FileOrganizer::ensureDefaultAssets() {
  // create if not exists
  QStringList assets = { "themes/dark.qss", "themes/light.qss", "patchnotes.md" };
  for (const QString& a : std::as_const(assets)) {
    if (!assetExists(a)) downloadAsset(a);
  }

  // checksum comparison
  QStringList downloadAssets = _avalidator.compareChecksums();

  if (downloadAssets.empty()) {
    fsinfo << "All external assets are up to date";
    return true;
  }

  for (const QString& rpath : std::as_const(downloadAssets)) {
    fsinfo << "New version of " + rpath + " found, downloading...";
    if (!downloadAsset(rpath)) {
      fserr << "Asset " + rpath + " couldn't be downloaded!";
    }
  }

  return true;
}

bool FileOrganizer::saveJson(const QString& filepath, const QJsonObject& obj) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    fswrn << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(obj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

bool FileOrganizer::saveJson(const QString& filepath, const QJsonArray& obj) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    fswrn << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(obj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

#include "versionmanager.h"
bool FileOrganizer::updateAvailable() {
  QString url = QString("https://raw.githubusercontent.com/%1/%2/%3/latest.ver")
                    .arg(GITHUB_USER, GITHUB_REPO, EXTERNAL_ASSETS_BRANCH);
  QNetworkAccessManager manager;
  QNetworkRequest req{QUrl(url)};
  req.setRawHeader("User-Agent", "UTB/2.0");
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                   QNetworkRequest::NoLessSafeRedirectPolicy);

  QNetworkReply* reply = manager.get(req);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    fserr << "Error for URL " << url << ": " << reply->errorString();
    reply->deleteLater();
    return false;
  }

  QByteArray data = reply->readAll();
  // data like this: 1.0.0 or 1.4.2
  Version remoteVersion = Version::fromString(QString(data));
  return _verman.compareVersions(remoteVersion, _verman.getVersion()) == 1;
}
