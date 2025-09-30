#ifndef ENUMS_H
#define ENUMS_H

#include <QString>
#include <QHash>

enum class Language { ENGLISH, TURKISH, GERMAN, FRENCH, ITALIAN, SPANISH, RUSSIAN };

enum class ToastType { INFO, ERROR, WARNING, SUCCESS };

enum class TreeItemType { NONE, REMOVE, FAVORITE };

enum class ToolCategory { UNIT_CONVERTERS, DEV_TOOLS, CRYPTORS, FILE_CONVERTERS, FIXERS };

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

enum class Weight {
  MILLIGRAMS,
  GRAMS,
  KILOGRAMS,
  TONS,
  POUNDS,
  OUNCE
};

enum class Currency {
  USD, // Amerikan Doları
  EUR, // Euro
  GBP, // İngiliz Sterlini
  TRY, // Türk Lirası
  JPY, // Japon Yeni
  CHF, // İsviçre Frangı
  CAD, // Kanada Doları
  AUD, // Avustralya Doları
  CNY, // Çin Yuanı
};

enum class UnitConverterType {
  LENGTH,
  WEIGHT,
  TIME,
  TEMPERATURE,
  CURRENCY
};

enum class CryptorAlgorithm {
  AES, RSA, SHA1, SHA224, SHA256, SHA384, SHA512, MD5, BASE64
};

enum class CryptorTools {
  AESRSA, HASHER, BASECODER
};

enum class GroupIcons {
  FOLDER, CODE, KEY, UNIT, WRENCH
};

inline GroupIcons getGroupIconEnum(const QString& codename) {
  if (codename == "files") return GroupIcons::FOLDER;
  if (codename == "code") return GroupIcons::CODE;
  if (codename == "key") return GroupIcons::KEY;
  if (codename == "unit") return GroupIcons::UNIT;
  if (codename == "wrench") return GroupIcons::WRENCH;

  return {};
}

inline QString getGroupIconString(GroupIcons gricon) {
  switch (gricon) {
    case GroupIcons::CODE: return "code";
    case GroupIcons::FOLDER: return "files";
    case GroupIcons::KEY: return "key";
    case GroupIcons::UNIT: return "unit";
    case GroupIcons::WRENCH: return "wrench";
    default: return "";
  }
}

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

inline Weight getWeightEnum(int index) {
  static const Weight enums[] = {
      Weight::MILLIGRAMS, Weight::GRAMS, Weight::KILOGRAMS, Weight::TONS, Weight::OUNCE, Weight::POUNDS
  };
  return (index >= 0 && index < 6) ? enums[index] : Weight::GRAMS;
}

inline Currency getCurrencyEnum(int index) {
  static const Currency enums[] = {
      Currency::USD,
      Currency::EUR,
      Currency::GBP,
      Currency::TRY,
      Currency::JPY,
      Currency::CHF,
      Currency::CAD,
      Currency::AUD,
      Currency::CNY,
      Currency::RUB,
      Currency::INR,
      Currency::SAR
  };
  return (index >= 0 && index < 12) ? enums[index] : Currency::USD;
}

// LANGUAGE ENUM
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
