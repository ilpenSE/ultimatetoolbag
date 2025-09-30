#ifndef UNITCONVERTER_H
#define UNITCONVERTER_H

#include <QString>
#include <QHash>
#include "../etc/enums.h"

class UnitConverter {
 public:
  UnitConverter();  
  // length
  // METRIC (MM, CM, DM, M, ... KM) <-> IMPERIAL (FEET, YARD, MILE ETC.)
  static double convertLength(double value, Length from, Length to);

  // time
  // MILLIS, SEC, MIN, HOURS
  static double convertTime(double value, Time from, Time to);

  // temperature
  // CELCIUS, FAHRENHEIT, KELVIN
  static double convertTemperature(double value, Temperature from, Temperature to);

  // weight
  // KG, MG, G, LBS, OZ, T
  static double convertWeight(double value, Weight from, Weight to);

  // currency, with api
  static double convertCurrency(double value, Currency from, Currency to);
};

#endif  // UNITCONVERTER_H
