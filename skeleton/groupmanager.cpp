#include "groupmanager.h"
#include "../etc/instances.h"
#include "loggerstream.h"

QSet<QString> GroupManager::toolKeys = {
    "pdfword", "videoaudio", "ziprar",
    "jsonprettier", "curl", "hashgen", "aesrsa", "baseencoder", "colorpicker", "regextester", "apitester",
    "length", "time", "weight", "temperature", "currency",
    "imagefixer", "jsonfixer"
};

QJsonObject GroupManager::defaultSchema = {
    { "File Converter", QJsonArray{ "pdfword", "videoaudio", "ziprar" } },
    { "Dev Tools", QJsonArray{ "jsonprettier", "curl", "hashgen", "aesrsa", "baseencoder", "colorpicker", "regextester", "apitester" } },
    { "Unit Converter", QJsonArray{ "length", "time", "weight", "temperature", "currency" } },
    { "Repairers", QJsonArray{ "imagefixer", "jsonfixer" } }
};

bool GroupManager::ensureJsonFile(QString* error) {
  if (!QFile::exists(jsonDir)) createJson();

  QJsonObject loadedJson = loadJson();
  if (loadedJson.isEmpty()) {
    *error = "Loaded groups json is empty!";
    return false;
  }
  QString errorM;
  QJsonObject saferJson = getSafeJson(loadedJson, &errorM);

  if (!errorM.isEmpty() || saferJson.empty()) {
    *error = errorM;
    return false;
  }

  groupsJson = saferJson;
  fsinfo << "Groups.json initialized and validated successfully!";
  return true;
}

bool GroupManager::createJson() {
  fsinfo << "Creating groups.json file";
  return _forg.saveJson(jsonDir, defaultSchema);
}

QJsonObject GroupManager::loadJson() {
  return _forg.loadJson(jsonDir);
}

QJsonObject GroupManager::getSafeJson(const QJsonObject& loadedJson, QString* error) {
  QJsonObject result;

  for (auto it = defaultSchema.begin(); it != defaultSchema.end(); ++it) {
    QString key = it.key();
    QJsonArray defaultArray = it.value().toArray();

    QJsonArray finalArray;
    bool keyValid = true;

    if (loadedJson.contains(key) && loadedJson[key].isArray()) {
      QJsonArray loadedArray = loadedJson[key].toArray();

      if (loadedArray.size() != defaultArray.size()) {
        keyValid = false;
      } else {
        for (int i = 0; i < loadedArray.size(); ++i) {
          if (loadedArray[i].toString() != defaultArray[i].toString()) {
            keyValid = false;
            break;
          }
        }
      }

      finalArray = keyValid ? loadedArray : defaultArray;

    } else {
      keyValid = false;
      finalArray = defaultArray;
    }

    if (!keyValid && error) {
      *error += QString("Key '%1' had invalid array, replaced with default.\n").arg(key);
    }

    result[key] = finalArray;
  }

  return result;
}

bool GroupManager::existsGroup(const QString& group) {
  return groupsJson.contains(group);
}

QStringList GroupManager::getGroupEntries(const QString& group) {
  if (!existsGroup(group)) {
    err << "Cannot get group " + group + "'s entries!";
    return {};
  }
  QJsonArray jsonArray = groupsJson[group].toArray();
  QStringList result;
  result.reserve(jsonArray.size());

  for (int i = 0; i < jsonArray.size(); ++i) {
    result.append(jsonArray[i].toString());
  }

  return result;
}

// may be deleted
bool GroupManager::setGroupEntries(const QString& group, QJsonArray arr) {
  if (!existsGroup(group)) {
    err << "Group named " + group + " couldn't be found while setting its entries";
    return false;
  }

  groupsJson[group] = arr;
  return true;
}

void GroupManager::saveOnExit() {
  _forg.saveJson(jsonDir, groupsJson);
}
