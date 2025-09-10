#ifndef ETAGCONTROLLER_H
#define ETAGCONTROLLER_H

#include <QJsonObject>
#include <QObject>
#include "appdatamanager.h"
#include "consts.h"

class EtagController : public QObject {
  Q_OBJECT
 public:
  bool ensureEtags();
  QStringList compareEtags();

  QJsonObject getEtags() { return etagsJson; }
  QJsonObject fetchEtags(const QMap<QString, QString>& urls, bool c = false);

  QString getKeyFromRelativePath(const QString& relativePath);
  bool updateEtagCache(const QString& key, const QString& newEtag);

  static EtagController& instance() {
    static EtagController _instance;
    return _instance;
  }

  QMap<QString, QString> getUrlMap() {
    return urlmap;
  }
  void updateUrlMaps();

 private:
  bool createJson();
  bool cacheJson();
  bool checkJson();

  QString etagsPath = AppDataManager::instance().appFolderPath() + "/etags.json";
  QJsonObject etagsJson;
  QStringList etags;

  QStringList defaultEtagsJsonKeys = { "patchnotes", "t_dark", "t_light"};
  QString urlHeader = QString("https://raw.githubusercontent.com/%1/%2/%3/")
                          .arg(GITHUB_USER, GITHUB_REPO, EXTERNAL_ASSETS_BRANCH);


  // URL: JSON_NAME
  QMap<QString, QString> urlmap;
  QMap<QString, QString> rurlmap;
  explicit EtagController(QObject *parent = nullptr) : QObject(parent) { updateUrlMaps(); };

  EtagController(const EtagController&) = delete;
  EtagController& operator=(const EtagController&) = delete;

  ~EtagController() override = default;

};

#endif  // ETAGCONTROLLER_H
