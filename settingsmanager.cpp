#include "settingsmanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "LoggerStream.h"
#include "consts.h"
#include "logger.h"

// Tüm ayarları kontrol eder ve düzeltir - SADECE eksik olanları ekle, mevcut
// değerleri değiştirme!
void SettingsManager::validateAndFixSettings(QJsonObject& settingsObj) {
  QJsonObject defaults = defaultSettings();

  // Tanınmayan key'leri sil
  for (auto it = settingsObj.begin(); it != settingsObj.end();) {
    if (!defaults.contains(it.key())) {
      swrn() << "Undefined key found: " + it.key() + ", deleting it.";
      it = settingsObj.erase(it);
    } else {
      ++it;
    }
  }

  // ÖNEMLİ: SADECE eksik değerleri ekle, mevcut değerleri DOKUNMA!
  for (auto it = defaults.begin(); it != defaults.end(); ++it) {
    if (!settingsObj.contains(it.key())) {
      settingsObj[it.key()] = it.value();
      swrn() << "The key " + it.key() + " cannot be found, adding it";
    }
  }

  // Version kontrolü: sadece version güncelle, diğer hiçbir şeye dokunma
  if (settingsObj.contains("Version") && settingsObj["Version"].isString()) {
    QString verinfile = settingsObj["Version"].toString();
    if (verinfile != APP_VERSION) {
      settingsObj["Version"] = APP_VERSION;
      swrn() << "Updating settings file version from " + verinfile + " to " +
                    APP_VERSION;
    }
  } else {
    settingsObj["Version"] = APP_VERSION;
    swrn() << "Settings file version cannot be found, setting it to " +
                  APP_VERSION;
  }

  sinfo() << "Settings validated. Only missing keys added and version updated.";
}

// Dosyadan ayarları okur, parse eder ve döner
// Başarılıysa ok true olur, aksi halde boş QJsonObject döner
QJsonObject SettingsManager::loadSettings(const QString& path, bool& ok) {
  ok = false;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    fserr() << "Settings file cannot be opened: " + path;
    return QJsonObject();
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
    fserr() << "Settings file parse error: " + parseError.errorString();
    return QJsonObject();
  }

  ok = true;
  return doc.object();
}

// Ayarları dosyaya kaydeder
bool SettingsManager::saveSettings(const QString& path,
                                   const QJsonObject& settingsObj) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    fserr() << "Settings file cannot be written: " + path;
    return false;
  }

  QJsonDocument doc(settingsObj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();

  fsinfo() << "Settings saved to: " + path;
  return true;
}
