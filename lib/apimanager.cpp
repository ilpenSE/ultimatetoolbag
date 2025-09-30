#include "apimanager.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonArray>

APIManager::APIManager(QObject *parent) : QObject(parent) {
  manager = new QNetworkAccessManager(this);
}

void APIManager::setApiConfig(const ApiConfig &cfg) {
  config = cfg;
}

void APIManager::setApiKey(const QString &key, AuthType authType) {
  config.apiKey = key;
  config.authType = authType;
}

void APIManager::setDefaultHeaders(const QVariantMap &headers) {
  config.defaultHeaders = headers;
}

void APIManager::setDefaultParams(const QVariantMap &params) {
  config.defaultParams = params;
}

void APIManager::addDefaultHeader(const QString &name, const QVariant &value) {
  config.defaultHeaders[name] = value;
}

void APIManager::addDefaultParam(const QString &name, const QVariant &value) {
  config.defaultParams[name] = value;
}

QNetworkRequest APIManager::buildRequest(const QString &url,
                                         const QVariantMap &params,
                                         const QVariantMap &headers) {
  QUrl qurl(url);
  QUrlQuery query(qurl);

  // Varsayılan parametreleri ekle
  for (auto it = config.defaultParams.begin(); it != config.defaultParams.end(); ++it) {
    query.addQueryItem(it.key(), it.value().toString());
  }

  // Ekstra parametreleri ekle
  for (auto it = params.begin(); it != params.end(); ++it) {
    query.addQueryItem(it.key(), it.value().toString());
  }

  // API key'i query parameter olarak ekle
  if (!config.apiKey.isEmpty() && config.authType == AuthType::QueryParam) {
    query.addQueryItem(config.keyParam, config.apiKey);
  }

  qurl.setQuery(query);
  QNetworkRequest request(qurl);

  // Varsayılan headerları ekle
  for (auto it = config.defaultHeaders.begin(); it != config.defaultHeaders.end(); ++it) {
    request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
  }

  // Ekstra headerları ekle
  for (auto it = headers.begin(); it != headers.end(); ++it) {
    request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
  }

  // API key'i header olarak ekle
  if (!config.apiKey.isEmpty() && config.authType == AuthType::Header) {
    QString headerValue = config.headerPrefix + config.apiKey;
    request.setRawHeader(config.headerName.toUtf8(), headerValue.toUtf8());
  }

  // Basic Auth
  if (!config.apiKey.isEmpty() && config.authType == AuthType::BasicAuth) {
    // Basic auth için genellikle username:password formatında encode edilir
    // Burada API key'i password olarak, username'i boş olarak kullanıyoruz
    QString credentials = ":" + config.apiKey;
    QByteArray encoded = credentials.toUtf8().toBase64();
    request.setRawHeader("Authorization", "Basic " + encoded);
  }

  // Varsayılan Content-Type
  if (!request.hasRawHeader("Content-Type")) {
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  }

  // User-Agent
  if (!request.hasRawHeader("User-Agent")) {
    request.setRawHeader("User-Agent", "UTB APIManager/1.0");
  }

  return request;
}

void APIManager::handleReply(QNetworkReply *reply,
                             std::function<void(const QJsonObject &)> onJsonSuccess,
                             std::function<void(const QByteArray &)> onRawSuccess,
                             std::function<void(const QString &)> onError) {

  connect(reply, &QNetworkReply::finished, this, [=]() {
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray data = reply->readAll();

      // Raw callback varsa onu çağır
      if (onRawSuccess) {
        onRawSuccess(data);
        emit rawResponseReceived(data);
      }

      // JSON callback varsa JSON parse et
      if (onJsonSuccess) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
          QJsonObject obj = doc.object();
          onJsonSuccess(obj);
          emit requestFinished(obj);
        } else if (doc.isArray()) {
          // Array gelirse object içine wrap et
          QJsonObject wrapper;
          wrapper["data"] = doc.array();
          onJsonSuccess(wrapper);
          emit requestFinished(wrapper);
        } else {
          // JSON parse edilemezse error
          QString errorMsg = "Invalid JSON response";
          if (onError) onError(errorMsg);
          emit requestError(errorMsg);
        }
      }
    } else {
      QString errorMsg = reply->errorString();

      // HTTP status code'u da ekle
      int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      if (httpStatus > 0) {
        errorMsg += QString(" (HTTP %1)").arg(httpStatus);
      }

      // Response body'de hata mesajı varsa onu da ekle
      QByteArray errorData = reply->readAll();
      if (!errorData.isEmpty()) {
        QJsonDocument errorDoc = QJsonDocument::fromJson(errorData);
        if (errorDoc.isObject()) {
          QJsonObject errorObj = errorDoc.object();
          if (errorObj.contains("message")) {
            errorMsg += ": " + errorObj["message"].toString();
          } else if (errorObj.contains("error")) {
            errorMsg += ": " + errorObj["error"].toString();
          }
        }
      }

      if (onError) onError(errorMsg);
      emit requestError(errorMsg);
    }
    reply->deleteLater();
  });
}

void APIManager::get(const QString &url,
                     std::function<void(const QJsonObject &)> onSuccess,
                     std::function<void(const QString &)> onError,
                     const QVariantMap &params,
                     const QVariantMap &headers) {

  QNetworkRequest request = buildRequest(url, params, headers);
  QNetworkReply *reply = manager->get(request);
  handleReply(reply, onSuccess, nullptr, onError);
}

void APIManager::post(const QString &url,
                      const QJsonObject &data,
                      std::function<void(const QJsonObject &)> onSuccess,
                      std::function<void(const QString &)> onError,
                      const QVariantMap &headers) {

  QNetworkRequest request = buildRequest(url, QVariantMap(), headers);
  QJsonDocument doc(data);
  QNetworkReply *reply = manager->post(request, doc.toJson());
  handleReply(reply, onSuccess, nullptr, onError);
}

void APIManager::put(const QString &url,
                     const QJsonObject &data,
                     std::function<void(const QJsonObject &)> onSuccess,
                     std::function<void(const QString &)> onError,
                     const QVariantMap &headers) {

  QNetworkRequest request = buildRequest(url, QVariantMap(), headers);
  QJsonDocument doc(data);
  QNetworkReply *reply = manager->put(request, doc.toJson());
  handleReply(reply, onSuccess, nullptr, onError);
}

void APIManager::deleteRequest(const QString &url,
                               std::function<void(const QJsonObject &)> onSuccess,
                               std::function<void(const QString &)> onError,
                               const QVariantMap &headers) {

  QNetworkRequest request = buildRequest(url, QVariantMap(), headers);
  QNetworkReply *reply = manager->deleteResource(request);
  handleReply(reply, onSuccess, nullptr, onError);
}

void APIManager::getRaw(const QString &url,
                        std::function<void(const QByteArray &)> onSuccess,
                        std::function<void(const QString &)> onError,
                        const QVariantMap &params,
                        const QVariantMap &headers) {

  QNetworkRequest request = buildRequest(url, params, headers);
  QNetworkReply *reply = manager->get(request);
  handleReply(reply, nullptr, onSuccess, onError);
}
