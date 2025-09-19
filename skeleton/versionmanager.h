#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QObject>
#include <QDebug>

struct Version {
  int major, minor, patch;

  QString toString() const {
    return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);
  }

  // parses version string to struct
  static Version fromString(const QString& ver) {
    Version v;
    QStringList nums = ver.split(".");
    if (nums.size() > 0) v.major = nums[0].toInt();
    if (nums.size() > 1) v.minor = nums[1].toInt();
    if (nums.size() > 2) v.patch = nums[2].toInt();
    return v;
  }

  // equality
  bool operator==(const Version& other) const {
    return major == other.major &&
           minor == other.minor &&
           patch == other.patch;
  }

  bool operator!=(const Version& other) const {
    return !(*this == other);
  }

  // greater-smaller
  bool operator<(const Version& other) const {
    if (major != other.major) return major < other.major;
    if (minor != other.minor) return minor < other.minor;
    return patch < other.patch;
  }

  bool operator>(const Version& other) const {
    return other < *this;
  }

  bool operator<=(const Version& other) const {
    return !(*this > other);
  }

  bool operator>=(const Version& other) const {
    return !(*this < other);
  }
};

/*
 * SEMANTIC VERSIONING:
 * major.minor.patch
 */
class VersionManager : public QObject {
  Q_OBJECT
 public:
  static VersionManager& instance() {
    static VersionManager _instance;
    return _instance;
  }

  void initVer(const QString& verstr) {
    m_version = Version::fromString(verstr);
  }

  Version getVersion() {
    return m_version;
  }

  // IF VER1 == VER2, RETURNS 0
  // IF VER1 > VER2, RETURNS 1
  // IF VER1 < VER2, RETURNS -1
  static int compareVersions(const Version& ver1, const Version& ver2) {
    // this operators overrided, equality's a bit faster so it goes first
    if (ver1 == ver2) return 0;
    if (ver1 > ver2) return 1;
    return -1;
  }

 private:
  explicit VersionManager(QObject* parent = nullptr) : QObject(parent) {}

  VersionManager(const VersionManager&) = delete;
  VersionManager& operator=(const VersionManager&) = delete;

  ~VersionManager() override = default;

  Version m_version;
};

#endif  // VERSIONMANAGER_H
