#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "fileorganizer.h"
class GroupManager : public QObject {
  Q_OBJECT
 public:
  static GroupManager& instance() {
    static GroupManager _instance;
    return _instance;
  }

  // safe save (more secure than destructor)
  void saveOnExit();
  // init and ensuring
  bool ensureJsonFile(QString* errorMessage = nullptr);

  // getter setter checkers
  QJsonObject getJson() { return groupsJson; };
  QStringList getGroups() { return groupsJson.keys(); };
  bool existsGroup(const QString& group);
  QStringList getGroupEntries(const QString& group);
  bool setGroupEntries(const QString& group, QJsonArray array);

  static QSet<QString> toolKeys;
  static QJsonObject defaultSchema;

 private:
  QString jsonDir = FileOrganizer::instance().appFolderPath() + "/groups.json";
  QJsonObject groupsJson;
  QJsonObject loadJson();
  bool createJson();
  bool checkJsonKeys(const QJsonObject& loadedJson);
  QJsonObject checkGroupObjects(const QJsonObject& loadedJson, QString* error = nullptr);

  explicit GroupManager(QObject *parent = nullptr) : QObject(parent) {};

  // block copy and assign
  GroupManager(const GroupManager&) = delete;
  GroupManager& operator=(const GroupManager&) = delete;

  ~GroupManager() override = default;
 signals:
};

#endif  // GROUPMANAGER_H
