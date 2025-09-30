#include "groupmanager.h"
#include "../lib/jsonworker.h"
#include "loggerstream.h"

QSet<QString> GroupManager::toolKeys = {
    "pdfword", "videoaudio", "ziprar",
    "jsonprettier", "curl", "hashgen", "aesrsa", "baseencoder", "colorpicker", "regextester", "apitester",
    "length", "time", "weight", "temperature", "currency",
    "imagefixer", "jsonfixer"
};

QJsonObject GroupManager::defaultSchema = {
    {
     "fileconverter", QJsonObject{
      {"name", "File Converter"},
      {"icon", "files"},
      {"entries", QJsonArray{ "pdfword", "videoaudio", "ziprar" }}
    }},
    {
     "devtools", QJsonObject{
      {"name", "Dev Tools"},
      {"icon", "code"},
      {"entries", QJsonArray{ "jsonprettier", "curl", "colorpicker", "regextester", "apitester" }}
    }},
    {
     "cryptors", QJsonObject{
        {"name", "Cryptors"},
        {"icon", "key"},
        {"entries", QJsonArray{ "hashgen", "aesrsa", "baseencoder" }}
    }},
    {
     "unitconverters", QJsonObject{
        {"name", "Unit Converters"},
        {"icon", "unit"},
        {"entries", QJsonArray{ "length", "time", "weight", "temperature", "currency" }}
      }},
    {
     "fixers", QJsonObject{
        {"name", "Fixers"},
        {"icon", "wrench"},
        {"entries", QJsonArray{ "imagefixer", "jsonfixer" }}
    }}
}; // names are user inputs

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
  return JSONWorker::saveJson(jsonDir, defaultSchema);
}

QJsonObject GroupManager::loadJson() {
  return JSONWorker::loadJson(jsonDir);
}

QJsonObject GroupManager::getSafeJson(const QJsonObject& loadedJson, QString* error) {
  QJsonObject result;

  for (auto it = defaultSchema.begin(); it != defaultSchema.end(); ++it) {
    QString key = it.key();
    QJsonObject defaultObj = it.value().toObject();

    QString defaultName = defaultObj["name"].toString();
    QString defaultIcon = defaultObj["icon"].toString();
    QJsonArray defaultEntries = defaultObj["entries"].toArray();

    QJsonObject finalObj;
    bool keyValid = true;

    if (loadedJson.contains(key) && loadedJson[key].isObject()) {
      QJsonObject loadedObj = loadedJson[key].toObject();

              // Check if all required fields exist and have correct types
      if (!loadedObj.contains("name") || !loadedObj["name"].isString() ||
          !loadedObj.contains("icon") || !loadedObj["icon"].isString() ||
          !loadedObj.contains("entries") || !loadedObj["entries"].isArray()) {
        keyValid = false;
      } else {
        QJsonArray loadedEntries = loadedObj["entries"].toArray();

        // Validate entries array
        if (loadedEntries.size() != defaultEntries.size()) {
          keyValid = false;
        } else {
          for (int i = 0; i < loadedEntries.size(); ++i) {
            if (loadedEntries[i].toString() != defaultEntries[i].toString()) {
              keyValid = false;
              break;
            }
          }
        }
      }

      finalObj = keyValid ? loadedObj : defaultObj;

    } else {
      keyValid = false;
      finalObj = defaultObj;
    }

    if (!keyValid && error) {
      *error += QString("Key '%1' had invalid structure, replaced with default.\n").arg(key);
    }

    result[key] = finalObj;
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
  JSONWorker::saveJson(jsonDir, groupsJson);
}
