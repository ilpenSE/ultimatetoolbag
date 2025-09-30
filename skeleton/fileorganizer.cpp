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

#include "../lib/jsonworker.h"

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
  }

  // version asset folder
  checkFolder(APP_VERSION, dir);

  // logs folder
  checkFolder("logs", dir);

  // cache folder
  checkFolder("cache", dir);

  return true;
}

void FileOrganizer::checkFolder(const QString& folder, QDir dir) {
  QString path = dir.filePath(folder);
  QDir fdir(path);
  if (!fdir.exists()) {
    if (fdir.mkpath(".")) {
      fsinfo << folder + " folder created: " + path;
    } else {
      fserr << folder + " folder cannot be created: " + path;
    }
  }
}

QString FileOrganizer::assetUrl(const QString& relativePath) {
  return QString("https://raw.githubusercontent.com/%1/%2/%3/%4/%5")
      .arg(GITHUB_USER, GITHUB_REPO, EXTERNAL_ASSETS_BRANCH, APP_VERSION, relativePath);
}

bool FileOrganizer::ensureThemesJson(const QString& themesPath, QString* error) {
  QString errm;
  QJsonObject themes = JSONWorker::loadJson(themesPath, &errm);

  if (!errm.isEmpty()) {
    fserr << errm;
    if (!JSONWorker::saveJson(themesPath, defaultThemesObj)) {
      fscrit << "Themes.json saving failed while creating";
      if (error) *error = "Themes.json saving failed while creating";
      return false;
    }
    fswrn << "Themes file was deleted or corrupted, created one.";
    themes = defaultThemesObj;
  } else {
    // add missing keys
    bool needsUpdate = false;
    for (auto it = defaultThemesObj.begin(); it != defaultThemesObj.end(); ++it) {
      if (!themes.contains(it.key())) {
        themes[it.key()] = it.value();
        thwrn << "The key " + it.key() + " cannot be found, adding it";
        needsUpdate = true;
      }
    }

    if (needsUpdate) {
      if (!JSONWorker::saveJson(themesPath, themes)) {
        fscrit << "Themes.json saving failed after updating";
        if (error) *error = "Themes.json saving failed after updating";
        return false;
      }
    }
  }

  fsinfo << "Themes.json validated successfully!";
  return true;
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

bool FileOrganizer::assetExists(const QString& relpath) {
  return QFile::exists(vAssetPath() + "/" + relpath);
}

bool FileOrganizer::ensureDefaultAssets() {
  // create if not exists
  QStringList assets = _avalidator.getRelativePaths();
  for (const QString& a : std::as_const(assets)) {
    if (!assetExists(a)) downloadAsset(a);
  }

  // checksum comparison
  QStringList downloadAssets = _avalidator.compareChecksums();

  if (downloadAssets.empty()) {
    fsinfo << "All external assets are up to date";
    return true;
  }

  // FIXME: patchnotes.md downloading every single time if there's exactly same as local
  for (const QString& rpath : std::as_const(downloadAssets)) {
    fsinfo << "New version of " + rpath + " found, downloading...";
    if (!downloadAsset(rpath)) {
      fserr << "Asset " + rpath + " couldn't be downloaded!";
    }
  }

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
