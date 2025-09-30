#include "ecbrates.h"
#include "../skeleton/loggerstream.h"
#include "../etc/instances.h"
#include <QFile>

void ECBRates::init() {
  linfo << "ECB Rates class has been initialized";
}

void ECBRates::onReply(QNetworkReply* reply) {
  if (reply->error() != QNetworkReply::NoError) {
    reply->deleteLater();
    return;
  }

  QMap<QString, double> rates;
  QString errm;
  if (!parseXml(reply->readAll(), rates, &errm)) {
    xmlerr << "Error while parsing ECB exchange XML: " + errm;
    reply->deleteLater();
    return;
  }

  reply->deleteLater();
  emit ratesReady(rates);
}

bool ECBRates::parseXml(const QString& xmlStr, QMap<QString, double>& rates, QString* errm) {
  QXmlStreamReader xml(xmlStr);

  while (!xml.atEnd() && !xml.hasError()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == "Cube") {
      auto attrs = xml.attributes();
      if (attrs.hasAttribute("currency") && attrs.hasAttribute("rate")) {
        QString currency = attrs.value("currency").toString();
        double rate = attrs.value("rate").toDouble();
        rates.insert(currency, rate);
      }
    }
  }

  if (xml.hasError()) {
    if (errm) *errm = xml.errorString();
    return false;
  }

  return true;
}
