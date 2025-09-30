#ifndef APIMANAGER_H
#define APIMANAGER_H

#include <QByteArray>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QVariantMap>
#include <functional>

enum class AuthType {
  None,
  QueryParam,     // ?api_key=xxx
  Header,         // Authorization: Bearer xxx veya X-API-Key: xxx
  BasicAuth       // Basic authentication
};

struct ApiConfig {
  QString apiKey;
  AuthType authType = AuthType::None;
  QString keyParam = "api_key";        // Query parameter adı (QueryParam için)
  QString headerName = "Authorization"; // Header adı (Header için)
  QString headerPrefix = "Bearer ";     // Header prefix (Header için)
  QVariantMap defaultHeaders;          // Varsayılan headerlar
  QVariantMap defaultParams;           // Varsayılan parametreler

  ApiConfig() = default;
  ApiConfig(const QString &key, AuthType type = AuthType::QueryParam)
      : apiKey(key), authType(type) {}
};

class APIManager : public QObject {
  Q_OBJECT

 public:
  static APIManager& instance() {
    static APIManager _instance;
    return _instance;
  }

  explicit APIManager(QObject *parent = nullptr);

  // API konfigürasyonu ayarlama
  void setApiConfig(const ApiConfig &config);
  void setApiKey(const QString &key, AuthType authType = AuthType::QueryParam);

  // Varsayılan header ve parametreleri ayarlama
  void setDefaultHeaders(const QVariantMap &headers);
  void setDefaultParams(const QVariantMap &params);
  void addDefaultHeader(const QString &name, const QVariant &value);
  void addDefaultParam(const QString &name, const QVariant &value);

  // HTTP methods
  void get(const QString &url,
           std::function<void(const QJsonObject &)> onSuccess = nullptr,
           std::function<void(const QString &)> onError = nullptr,
           const QVariantMap &params = QVariantMap(),
           const QVariantMap &headers = QVariantMap());

  void post(const QString &url,
            const QJsonObject &data,
            std::function<void(const QJsonObject &)> onSuccess = nullptr,
            std::function<void(const QString &)> onError = nullptr,
            const QVariantMap &headers = QVariantMap());

  void put(const QString &url,
           const QJsonObject &data,
           std::function<void(const QJsonObject &)> onSuccess = nullptr,
           std::function<void(const QString &)> onError = nullptr,
           const QVariantMap &headers = QVariantMap());

  void deleteRequest(const QString &url,
                     std::function<void(const QJsonObject &)> onSuccess = nullptr,
                     std::function<void(const QString &)> onError = nullptr,
                     const QVariantMap &headers = QVariantMap());

  // Raw response almak için (JSON olmayan cevaplar için)
  void getRaw(const QString &url,
              std::function<void(const QByteArray &)> onSuccess = nullptr,
              std::function<void(const QString &)> onError = nullptr,
              const QVariantMap &params = QVariantMap(),
              const QVariantMap &headers = QVariantMap());

 signals:
  void requestFinished(const QJsonObject &response);
  void requestError(const QString &error);
  void rawResponseReceived(const QByteArray &data);

 private:
  QNetworkAccessManager *manager;
  ApiConfig config;

  // Helpers
  QNetworkRequest buildRequest(const QString &url,
                               const QVariantMap &params = QVariantMap(),
                               const QVariantMap &headers = QVariantMap());
  void handleReply(QNetworkReply *reply,
                   std::function<void(const QJsonObject &)> onJsonSuccess,
                   std::function<void(const QByteArray &)> onRawSuccess,
                   std::function<void(const QString &)> onError);

  // block copy and assign
  APIManager(const APIManager&) = delete;
  APIManager& operator=(const APIManager&) = delete;

  ~APIManager() override = default;
};

#endif  // APIMANAGER_H
