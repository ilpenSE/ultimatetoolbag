#ifndef JSONWORKER_H
#define JSONWORKER_H

#include <QString>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class JSONWorker {
 public:
  JSONWorker();

  // JSON RW
  static QJsonDocument loadJsonDoc(const QString& filepath);
  static QJsonObject loadJson(const QString& filepath, QString* errm = nullptr);
  static QJsonArray loadJsonArr(const QString& filepath, QString* errm = nullptr);
  static bool saveJson(const QString& filepath, const QJsonObject& obj);
  static bool saveJson(const QString& filepath, const QJsonArray& obj);

  // JSON Prettier
  static QString prettify(const QString& json);

  // JSON Fixer
  static QString fix(const QString& json);
};

#endif  // JSONWORKER_H
