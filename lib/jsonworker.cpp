#include "jsonworker.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>

#include "../skeleton/loggerstream.h"

JSONWorker::JSONWorker() {}

QString JSONWorker::prettify(const QString& json) {
  return json;
}

QString JSONWorker::fix(const QString& json) {
  return json;
}

QJsonDocument JSONWorker::loadJsonDoc(const QString& filepath) {
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    jserr << "Json file cannot be opened: " + filepath;
    return {};
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    jserr << "Json file parse error: " + parseError.errorString();
    return {};
  }

  return doc;
}

QJsonObject JSONWorker::loadJson(const QString& filepath, QString* error) {
  QJsonDocument d = loadJsonDoc(filepath);
  if (!d.isObject()) if (error) *error = filepath + " is not an JSON object!";
  return loadJsonDoc(filepath).object();
}

QJsonArray JSONWorker::loadJsonArr(const QString& filepath, QString* error) {
  QJsonDocument d = loadJsonDoc(filepath);
  if (!d.isArray()) { if (error) *error = filepath + " is not an JSON array!"; }
  return loadJsonDoc(filepath).array();
}

bool JSONWorker::saveJson(const QString& filepath, const QJsonObject& obj) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    jswrn << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(obj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

bool JSONWorker::saveJson(const QString& filepath, const QJsonArray& obj) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    jswrn << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(obj);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}
