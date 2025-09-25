#ifndef ENUMS_H
#define ENUMS_H

#include <QString>
#include <QHash>

enum class Language { ENGLISH, TURKISH, GERMAN, FRENCH, ITALIAN, SPANISH, RUSSIAN };

enum class ToastType { INFO, ERROR, WARNING, SUCCESS };

enum class TreeItemType { NONE, REMOVE, FAVORITE };

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
