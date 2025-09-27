#include "unitconverter.h"
#include "../etc/enums.h"

UnitConverter::UnitConverter() {}

double UnitConverter::convertLength(double value, Length from, Length to) {
  // metre base
  static const QHash<Length, double> factors = {
      {Length::MILLIMETER, 0.001},
      {Length::CENTIMETER, 0.01},
      {Length::DECIMETER,  0.1},
      {Length::METER,      1.0},
      {Length::KILOMETER,  1000.0},
      {Length::INCH,       0.0254},
      {Length::FOOT,       0.3048},
      {Length::YARD,       0.9144},
      {Length::MILE,       1609.344}
  };
  return value * factors[from] / factors[to];
}

double UnitConverter::convertTime(double value, Time from, Time to) {
  // seconds base
  static const QHash<Time, double> factors = {
      {Time::MILLISECONDS, 0.001},
      {Time::SECONDS,      1.0},
      {Time::MINUTES,      60.0},
      {Time::HOURS,        3600.0}
  };
  return value * factors[from] / factors[to];
}

double UnitConverter::convertTemperature(double value, Temperature from, Temperature to) {
  double celsius;
  switch (from) {
    case Temperature::CELCIUS:    celsius = value; break;
    case Temperature::KELVIN:     celsius = value - 273.15; break;
    case Temperature::FAHRENHEIT: celsius = (value - 32.0) * 5.0 / 9.0; break;
  }

  switch (to) {
    case Temperature::CELCIUS:    return celsius;
    case Temperature::KELVIN:     return celsius + 273.15;
    case Temperature::FAHRENHEIT: return celsius * 9.0 / 5.0 + 32.0;
  }

  return value;
}
