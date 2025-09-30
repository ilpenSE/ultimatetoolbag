#include "unitconverter.h"
#include "../etc/enums.h"

#include "../etc/instances.h"

UnitConverter::UnitConverter() {
  _ecbrates.init();
}

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
  // celcius base
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

double UnitConverter::convertWeight(double value, Weight from, Weight to) {
  // grams base
  double grams;
  switch (from) {
    case Weight::GRAMS:     grams = value; break;
    case Weight::KILOGRAMS: grams = value * 1000.0; break;
    case Weight::MILLIGRAMS:grams = value / 1000.0; break;
    case Weight::TONS:      grams = value * 1000000.0; break;
    case Weight::OUNCE:     grams = value * 28.349523125; break;
    case Weight::POUNDS:    grams = value * 453.59237; break;
  }

  switch (to) {
    case Weight::GRAMS:     return grams;
    case Weight::KILOGRAMS: return grams / 1000.0;
    case Weight::MILLIGRAMS:return grams * 1000.0;
    case Weight::TONS:      return grams / 1000000.0;
    case Weight::OUNCE:     return grams / 28.349523125;
    case Weight::POUNDS:    return grams / 453.59237;
  }
  return 0.0;
}

double UnitConverter::convertCurrency(double value, Currency from, Currency to) {

  return 0.0;
}
