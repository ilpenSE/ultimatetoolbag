#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QObject>

/*
 * SÜRÜMLEME SİSTEMİ:
 * major.minor.patch
 */
class VersionManager : public QObject {
  Q_OBJECT
 public:
  static VersionManager& instance() {
    static VersionManager _instance;
    return _instance;
  }

  QString normalize(const QString& ver) {
    QString versionPart = ver;

    QStringList numbers = versionPart.split(".");
    // Eksik olan parçaları tamamla
    while (numbers.size() < 3) numbers.append("0");

    QString normalizedVersion = numbers.join(".");
    return normalizedVersion;
  }

  void initVer(const QString& verstr) { m_versionString = normalize(verstr); }

  QString version() const { return m_versionString; }

  int major() const {
    return m_versionString.split("-")[0].split(".")[0].toInt();
  }

  int minor() const {
    return m_versionString.split("-")[0].split(".")[1].toInt();
  }

  int patch() const {
    return m_versionString.split("-")[0].split(".")[2].toInt();
  }

 private:
  explicit VersionManager(QObject* parent = nullptr) : QObject(parent) {}

  VersionManager(const VersionManager&) = delete;
  VersionManager& operator=(const VersionManager&) = delete;

  ~VersionManager() override = default;

  QString m_versionString;
};

#endif  // VERSIONMANAGER_H
