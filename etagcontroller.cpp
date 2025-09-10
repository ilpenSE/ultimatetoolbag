#include "etagcontroller.h"
#include "appdatamanager.h"
#include "LoggerStream.h"

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

bool EtagController::ensureEtags() {
  if (!QFile::exists(etagsPath)) return createJson();

  if (!cacheJson()) {
    fserr() << "Etags json load failed!";
    return false;
  }

  if (!checkJson()) {
    fswrn() << "Etag json check failed!";
    return false;
  }
  return true;
}

bool EtagController::checkJson() {
  QStringList cached = etagsJson.keys();
  QStringList defs = defaultEtagsJsonKeys;
  defs.sort(Qt::CaseSensitive);
  cached.sort(Qt::CaseSensitive);

  if (cached == defs) {
    return true;
  }

  QJsonObject etagsObj = fetchEtags(urlmap);

  if (!AppDataManager::instance().saveJson(etagsPath, etagsObj)) {
    fserr() << "Cannot save etag json!";
    return false;
  }

  return true;
}

bool EtagController::cacheJson() {
  QFile file(etagsPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fserr() << "Etag file cannot be opened: " + etagsPath;
    return false;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    fserr() << "Etag file parse error: " + parseError.errorString();
    return false;
  }
  etagsJson = doc.object();
  for (auto it = etagsJson.begin(); it != etagsJson.end(); ++it) {
    etags.append(it.value().toString());
  }

  return true;
}

bool EtagController::createJson() {
  /*
   * etags.json:
   {
      "patchnotes": "some etag",
      "t_dark": "some etag",
      "t_light": "some etag"
   }
   */
  QJsonObject etagsObj = fetchEtags(urlmap, true);

  if (etagsObj.empty()) return false;

  if (!AppDataManager::instance().saveJson(etagsPath, etagsObj)) {
    fserr() << "Cannot create etags.json!";
    return false;
  }

  etagsJson = etagsObj;
  return true;
}

QJsonObject EtagController::fetchEtags(const QMap<QString, QString>& urls, bool c) {
  QJsonObject result;
  QNetworkAccessManager manager;

  for (auto it = urls.cbegin(); it != urls.cend(); ++it) {
    const QString &url = it.key();
    const QString &key = it.value();

    // github raw cdn yapılan commitleri anında görmez, biraz bekleme gerekir
    // anında değişikliği fark etmesi için github api kullan
    QNetworkRequest req{ QUrl(url) };
    req.setRawHeader("User-Agent", "UTB/2.0");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = manager.head(req);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
      fserr() << "Error for URL " << url << ": " << reply->errorString();
      reply->deleteLater();
      continue;
    }

    QString etag = reply->rawHeader("ETag");
    if (etag.isEmpty()) {
      fswrn() << "No Etag Found for URL " << url << ": " << reply->errorString();
      reply->deleteLater();
      continue;
    }

    if (c) {etags.append(etag);}
    result.insert(key, etag);
    reply->deleteLater();
  }

  return result;
}

QStringList EtagController::compareEtags() {
  QStringList downloadRelPaths;
  QJsonObject gitEtags = fetchEtags(urlmap);

  if (gitEtags.isEmpty()) {
    err() << "Could not fetch remote etags, skipping update check";
    return downloadRelPaths;
  }

  // patch notes etag check
  QString localPatchEtag = etagsJson["patchnotes"].toString();
  QString remotePatchEtag = gitEtags["patchnotes"].toString();

  if (remotePatchEtag != localPatchEtag) {
    downloadRelPaths << rurlmap["patchnotes"].mid(urlHeader.length());
  }

  // check etags except patchnotes
  QStringList keys = etagsJson.keys();
  keys.removeOne("patchnotes");
  for (const QString& key : std::as_const(keys)) {
    QString localEtag = etagsJson[key].toString();
    QString remoteEtag = gitEtags.contains(key) ? gitEtags[key].toString() : "NOT_FOUND";

    if (gitEtags.contains(key) && remoteEtag != localEtag) {
      QString relPath = rurlmap[key].mid(urlHeader.length());
      downloadRelPaths << relPath;
    }
  }

  return downloadRelPaths;
}

void EtagController::updateUrlMaps() {
  urlmap.clear();
  rurlmap.clear();

  // Patchnotes
  urlmap[urlHeader + "patchnotes.md"] = "patchnotes";
  rurlmap["patchnotes"] = urlHeader + "patchnotes.md";

  // Theme files
  urlmap[urlHeader + "themes/dark.qss"] = "t_dark";
  urlmap[urlHeader + "themes/light.qss"] = "t_light";
  rurlmap["t_dark"] = urlHeader + "themes/dark.qss";
  rurlmap["t_light"] = urlHeader + "themes/light.qss";
}

bool EtagController::updateEtagCache(const QString& key, const QString& newEtag) {
  etagsJson[key] = newEtag;
  return AppDataManager::instance().saveJson(etagsPath, etagsJson);
}

QString EtagController::getKeyFromRelativePath(const QString& relativePath) {
  if (relativePath == "patchnotes.md") return "patchnotes";
  if (relativePath.endsWith("themes/dark.qss")) return "t_dark";
  if (relativePath.endsWith("themes/light.qss")) return "t_light";
  return "";
}

