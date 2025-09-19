#include "assetvalidator.h"

#include <QCryptographicHash>
#include <QFile>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "fileorganizer.h"
#include "loggerstream.h"
#include "../etc/instances.h"

AssetValidator::AssetValidator(QObject *parent) : QObject(parent) {
  remoteHashes = fetchChecksums();
}

QString AssetValidator::calculateLocalChecksum(const QString& relpath) {
  QString fullPath = QDir(_forg.vAssetPath()).filePath(relpath);
  QFile file(fullPath);
  if (!file.open(QIODevice::ReadOnly)) return QString();

  QCryptographicHash hash(QCryptographicHash::Sha1);
  if (!hash.addData(&file)) return QString();

  return QString(hash.result().toHex());
}

QStringList AssetValidator::compareChecksums() {
  QStringList toDownload; // relative paths to download

  // Remote checksums.json
  for (auto it = remoteHashes.begin(); it != remoteHashes.end(); ++it) {
    QString key = it.key();
    QString remoteHash = it.value().toString();
    QString relPath = relpaths[key];
    QString localHash = calculateLocalChecksum(relPath);

    if (localHash.isEmpty() || localHash != remoteHash) {
      toDownload << relPath;
    }
  }
  return toDownload;
}

QJsonObject AssetValidator::fetchChecksums() {
  QString url = FileOrganizer::instance().assetUrl("checksums.json");
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
    return {};
  }

  QByteArray data = reply->readAll();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

  if (parseError.error != QJsonParseError::NoError) {
    serr << "Checksum JSON parse error:" << parseError.errorString();
    return {};
  }

  if (!doc.isObject()) {
    serr << "Checksum json is not an object";
    return {};
  }

  QJsonObject res = doc.object();

  if (res.keys() != defaultChecksumKeys) {
    for (const QString& key : std::as_const(defaultChecksumKeys)) {
      res.insert(key, res[key]);
    }
  }

  return res;
}
