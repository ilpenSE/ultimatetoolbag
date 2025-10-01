#include "jsonworker.h"
#include "nlohmannjson.hpp"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QIODevice>
#include <QJsonParseError>
#include "../skeleton/loggerstream.h"
#include <stack>

JSONWorker::JSONWorker() {}
using json = nlohmann::json;

QString JSONWorker::prettify(const QString& json) {
  QJsonParseError parseErr;
  QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseErr);
  if (parseErr.error != QJsonParseError::NoError) {
    // parse hatası varsa aynen döndür
    return json;
  }
  return doc.toJson(QJsonDocument::Indented);
}

QString JSONWorker::fix(const QString& jsonStr)  {
  std::string s = jsonStr.toStdString();
  std::string result;
  std::stack<char> parenStack;

  const std::unordered_map<char,char> parens = {
      {'{','}'}, {'[',']'}, {'(',')'}
  };

  auto isOpen = [&](char c){ return parens.count(c) > 0; };
  auto isClose = [&](char c){
    for (auto &p : parens) if (p.second == c) return true;
    return false;
  };
  auto match = [&](char open, char close){ return parens.at(open) == close; };

          // Tek geçişte parantez dengeleme ve string key fix
  for (size_t i=0; i<s.size(); ++i) {
    char c = s[i];
    // Key string fix: key: → "key":
    if ((i==0 || s[i-1]=='{' || s[i-1]==',' || s[i-1]=='\n' || s[i-1]==' ') &&
        isalpha(c)) {
      // capture alfanumerik + _
      size_t start = i;
      while (i<s.size() && (isalnum(s[i]) || s[i]=='_')) i++;
      if (i<s.size() && s[i]==':') {
        result += "\"" + s.substr(start, i - start) + "\":";
        continue;
      } else {
        i--; // geri dön
      }
    }

            // Tek tırnak → çift tırnak
    if (c=='\'') { c='"'; }

            // Parantez dengeleme
    if (isOpen(c)) {
      parenStack.push(c);
      result += c;
    } else if (isClose(c)) {
      if (!parenStack.empty() && match(parenStack.top(), c)) {
        parenStack.pop();
        result += c;
      } else {
        // Fazla kapalı parantez atla
        continue;
      }
    } else {
      result += c;
    }
  }

          // Stack’te kalan açık parantezleri kapat
  while (!parenStack.empty()) {
    result += parens.at(parenStack.top());
    parenStack.pop();
  }

          // Regex ile fazladan virgül temizleme
  std::string res2 = result;
  QRegularExpression rxTrailingComma(R"((,)\s*([\]\}]))");
  QString qs = QString::fromStdString(res2);
  qs.replace(rxTrailingComma, R"(\2)");

          // Son olarak parse edip prettify et
  try {
    json j = json::parse(qs.toStdString());
    return QString::fromStdString(j.dump(4));
  } catch (json::parse_error&) {
    return qs; // hâlâ parse edilemiyorsa düzeltmelerle dön
  }
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
    jserr << "Json file parse error: " + parseError.errorString() + ", in file: " + filepath;
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

bool JSONWorker::saveJson(const QString& filepath, const QJsonArray& arr) {
  QFile file(filepath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    jswrn << "Cannot write json to " << filepath;
    return false;
  }
  QJsonDocument doc(arr);
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}
