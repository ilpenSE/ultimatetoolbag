#include "groupmanager.h"
#include "../lib/jsonworker.h"
#include "loggerstream.h"
#include "../etc/enums.h"

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
  if (!QFile::exists(jsonDir)) return createJson();

  QJsonObject loadedJson = loadJson();
  if (loadedJson.isEmpty()) {
    *error = "Loaded groups json is empty!";
    return false;
  }
  QString errorM;
  QJsonObject saferJson;

  bool hasSameAsDefault = checkJsonKeys(loadedJson);
  if (!hasSameAsDefault) {
    saferJson = defaultSchema;
  } else {
    saferJson = checkGroupObjects(loadedJson, &errorM);

    if (!errorM.isEmpty() || saferJson.empty()) {
      *error = errorM;
      return false;
    }
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

QJsonObject GroupManager::checkGroupObjects(const QJsonObject& loadedJson, QString* error) {
  QJsonObject result;

  for (auto it = defaultSchema.begin(); it != defaultSchema.end(); ++it) {
    QString groupName = it.key(); // eg: fileconverter
    QJsonObject defaultObj = it.value().toObject(); // eg: {"name": "File Converter", "icon":"...", "entries": ["", "", ...]}
    QJsonArray defaultEntries = defaultObj["entries"].toArray();

    QJsonObject finalObj;
    bool keyValid = true;

    if (loadedJson[groupName].isObject()) {
      QJsonObject loadedObj = loadedJson[groupName].toObject();

      // Check if all required fields exist and have correct types
      bool isEntriesValid = loadedObj.contains("entries") && loadedObj["entries"].isArray();
      bool isNameValid = loadedObj.contains("name") && loadedObj["name"].isString();
      bool isIconValid = loadedObj.contains("icon") && loadedObj["icon"].isString();

      // name control
      // name must be 1 to 20 chars long, a-Z, 0-9, and spaces
      if (isNameValid) {
        QString namestr = loadedObj["name"].toString();
        static QRegularExpression rx(R"(^[\p{L}\p{N} ]{1,20}$)");
        if (!rx.match(namestr).hasMatch()) {
          isNameValid = false;
        }
      }

      // is icon a registered icon
      if (isIconValid) isIconValid &= _groupIconsHash.contains(loadedObj["icon"].toString());

      keyValid = isNameValid && isIconValid;

      if (!isEntriesValid) keyValid = loadedObj["entries"].toArray() == defaultEntries;

      finalObj = keyValid ? loadedObj : defaultObj;

    } else {
      keyValid = false;
      finalObj = defaultObj;
    }

    if (!keyValid) {
      warn << QString("Key '%1' had invalid structure, replaced with default.\n").arg(groupName);
    }

    result[groupName] = finalObj;
  }

  return result;
}

bool GroupManager::checkJsonKeys(const QJsonObject& loadedJson) {
  QStringList groupnames = loadedJson.keys();
  return toolKeys == QSet<QString>(groupnames.begin(), groupnames.end());
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
