#ifndef ASSETVALIDATOR_H
#define ASSETVALIDATOR_H

#include <QObject>
#include <QJsonObject>
#include "../etc/consts.h"

class AssetValidator : public QObject {
  Q_OBJECT
 public:
  static AssetValidator& instance() {
    static AssetValidator _instance;
    return _instance;
  }

  explicit AssetValidator(QObject *parent = nullptr);

  QStringList compareChecksums();

  QString calculateLocalChecksum(const QString& relpath);
  QJsonObject fetchChecksums();
 private:
  QStringList defaultChecksumKeys = {"patchnotes", "t_dark", "t_light", "t_synthwave", "t_singularity"};
  QMap<QString, QString> relpaths = {
      {"patchnotes", "patchnotes.md"},
      {"t_dark", "themes/dark.qss"},
      {"t_light", "themes/light.qss"},
      {"t_synthwave", "themes/synthwave.qss"},
      {"t_singularity", "themes/singularity.qss"}
  }; // remote key -> relative path
  QJsonObject remoteHashes;

  QString urlHeader =
      QString("https://raw.githubusercontent.com/%1/%2/%3/%4/")
          .arg(GITHUB_USER, GITHUB_REPO, EXTERNAL_ASSETS_BRANCH, APP_VERSION);
 signals:
};

#endif  // ASSETVALIDATOR_H
