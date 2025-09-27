#include "favoritesmanager.h"
#include "../etc/instances.h"
#include "loggerstream.h"
#include <QSet>
#include "../lib/jsonworker.h"

FavoritesManager::FavoritesManager(QObject *parent) : QObject(parent) {
  jsonPath = _forg.appFolderPath() + "/favs.json";
}

// O(n) solution
static QJsonArray removeDuplicates(const QJsonArray& arr) {
  QSet<QString> seen;
  QJsonArray result;
  for (const auto& val : arr) {
    QString s = val.toVariant().toString();
    if (!seen.contains(s)) {
      seen.insert(s);
      result.append(s);
    }
  }
  return result;
}

bool FavoritesManager::ensureFavs() {
  if (!QFile::exists(jsonPath)) return createJson();
  QString errM;
  QJsonArray loadedarr = JSONWorker::loadJsonArr(jsonPath, &errM);

  if (!errM.isEmpty()) {
    fserr << errM;
    return false;
  }

  QSet<QString> alltools = _groupman.toolKeys;

  QJsonArray filtered;
  for (const auto& v : std::as_const(loadedarr)) {
    if (!v.isString()) continue;
    QString key = v.toString();
    if (alltools.contains(key)) {
      filtered.append(key);
    }
  }

  favsJson = removeDuplicates(filtered);
  return true;
}

bool FavoritesManager::addFavorite(const QString& tool, QString* error) {
  if (!_groupman.toolKeys.contains(tool)) {
    if (error) *error = "This tool isn't valid";
    return false;
  }

  if (existsFavorite(tool)) {
    if (error) *error = "This tool already exists!";
    return false;
  }

  favsJson.append(tool);
  return true;
}

bool FavoritesManager::removeFavorite(const QString& fav, QString* error) {
  for (int i = 0; i < favsJson.size(); ++i) {
    if (favsJson.at(i).toString() == fav) {
      favsJson.removeAt(i);
      return true;
    }
  }
  if (error) *error = "Favorite tool named " + fav + " doesn't exist.";
  return false;
}

QJsonArray FavoritesManager::getFavorites() {
  return favsJson;
}

bool FavoritesManager::existsFavorite(const QString& tool) {
  return favsJson.contains(tool);
}

bool FavoritesManager::createJson() {
  if (!JSONWorker::saveJson(jsonPath, QJsonArray{})) {
    fserr << "Cannot create favs.json!";
    return false;
  }

  fsinfo << "Favs.json successfully created!";
  return true;
}

void FavoritesManager::saveOnExit() {
  if (!JSONWorker::saveJson(jsonPath, favsJson)) {
    fserr << "Cannot save favs.json!";
  }
}
