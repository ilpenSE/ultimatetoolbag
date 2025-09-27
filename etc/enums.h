#ifndef ENUMS_H
#define ENUMS_H

#include <QString>
#include <QHash>

enum class Language { ENGLISH, TURKISH, GERMAN, FRENCH, ITALIAN, SPANISH, RUSSIAN };

enum class ToastType { INFO, ERROR, WARNING, SUCCESS };

enum class TreeItemType { NONE, REMOVE, FAVORITE };

enum class Temperature { CELCIUS, KELVIN, FAHRENHEIT };

enum class Length {
  // Metric
  MILLIMETER,
  CENTIMETER,
  DECIMETER,
  METER,
  KILOMETER,
  // Imperial
  INCH,
  FOOT,
  YARD,
  MILE
};

enum class Time { MILLISECONDS, SECONDS, MINUTES, HOURS };

// index -> enum
inline Length getLengthEnum(int index) {
  static const Length enums[] = {
      Length::MILLIMETER, Length::CENTIMETER, Length::DECIMETER,
      Length::METER, Length::KILOMETER, Length::INCH,
      Length::FOOT, Length::YARD, Length::MILE
  };
  return (index >= 0 && index < 9) ? enums[index] : Length::METER;
}

inline Time getTimeEnum(int index) {
  static const Time enums[] = {
      Time::MILLISECONDS, Time::SECONDS, Time::MINUTES, Time::HOURS
  };
  return (index >= 0 && index < 4) ? enums[index] : Time::SECONDS;
}

inline Temperature getTemperatureEnum(int index) {
  static const Temperature enums[] = {
      Temperature::CELCIUS, Temperature::KELVIN, Temperature::FAHRENHEIT
  };
  return (index >= 0 && index < 3) ? enums[index] : Temperature::CELCIUS;
}

inline QString getLanguageStr(Language lang) {
  switch(lang) {
    case Language::ENGLISH: return "English";
    case Language::TURKISH: return "Türkçe";
    case Language::GERMAN: return "Deutsch";
    case Language::FRENCH: return "Français";
    case Language::ITALIAN: return "Italiano";
    case Language::SPANISH: return "Español";
    case Language::RUSSIAN: return "Русский";
    default: return "";
  }
}

inline QString getLanguageStr(const QString& langcode) {
  static const QHash<QString, QString> map = {
      {"en_US", "English"},
      {"tr_TR", "Türkçe"},
      {"de_DE", "Deutsch"},
      {"fr_FR", "Français"},
      {"it_IT", "Italiano"},
      {"es_ES", "Español"},
      {"ru_RU", "Русский"}
  };

  return map.value(langcode, "");
}

#endif  // ENUMS_H
