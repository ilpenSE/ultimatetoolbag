#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QObject>
#include <QTranslator>

#include "enums.h"

class LanguageManager : public QObject {
  Q_OBJECT
 public:
  static LanguageManager &instance() {
    static LanguageManager _instance;
    return _instance;
  }

  const QHash<Language, QString> languageToLocaleMap = {
      {Language::ENGLISH, "en_US"},
      {Language::TURKISH, "tr_TR"},
      {Language::GERMAN, "de_DE"},
      {Language::FRENCH, "fr_FR"},
      {Language::ITALIAN, "it_IT"},
      {Language::SPANISH, "es_ES"}};

  const QHash<QString, Language> localeToLanguageMap = {
      {"en_US", Language::ENGLISH},
      {"tr_TR", Language::TURKISH},
      {"de_DE", Language::GERMAN},
      {"fr_FR", Language::FRENCH},
      {"it_IT", Language::ITALIAN},
      {"es_ES", Language::SPANISH}};

  bool changeLanguage(const QString &languageCode);
  bool changeLanguage(const Language &language);

  bool loadLanguage(const QString &languageCode = QString());
  bool loadLanguage(const Language &lang);

  QString getsyslang();
  QString getCurrentLanguageStr();
  Language getCurrentLanguage();
  QTranslator *translator() { return &m_translator; }

  Language localeToLanguage(const QString &locale);
  QString languageToLocale(Language lang);

 signals:
  void languageChanged();

 private:
  explicit LanguageManager(QObject *parent = nullptr) : QObject(parent) {}
  QTranslator m_translator;
  QString m_currentLocale;  // Mevcut locale'i takip etmek için

  // Copy ve assign engelle
  LanguageManager(const LanguageManager &) = delete;
  LanguageManager &operator=(const LanguageManager &) = delete;

  ~LanguageManager() override = default;
};

#endif  // LANGUAGEMANAGER_H
