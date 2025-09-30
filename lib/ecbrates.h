#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QMap>

class ECBRates : public QObject {
  Q_OBJECT
 public:
  static ECBRates& instance() {
    static ECBRates _ins;
    return _ins;
  }

  void init();
  explicit ECBRates(QObject *parent = nullptr) : QObject(parent) {
    connect(&manager, &QNetworkAccessManager::finished, this, &ECBRates::onReply);
  }

  void fetch() {
    QUrl url("https://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml");
    manager.get(QNetworkRequest(url));
  }

 signals:
  void ratesReady(const QMap<QString, double>& rates);

 private slots:
  void onReply(QNetworkReply* reply);

 private:
  QNetworkAccessManager manager;

  bool parseXml(const QString& xmlStr, QMap<QString, double>& rates, QString* errorMessage = nullptr);
};
