#include "languagemanager.h"

#include <QApplication>
#include <QDebug>
#include <QFile>

#include "loggerstream.h"
#include "../etc/enums.h"
#include "settingsmanager.h"

const QHash<Language, QString> LanguageManager::languageToLocaleMap = {
    {Language::ENGLISH, "en_US"}, {Language::TURKISH, "tr_TR"},
    {Language::GERMAN, "de_DE"},  {Language::FRENCH, "fr_FR"},
    {Language::ITALIAN, "it_IT"}, {Language::SPANISH, "es_ES"},
    {Language::RUSSIAN, "ru_RU"}};

const QList<Language> LanguageManager::orderedLanguages = {
    Language::ENGLISH, Language::TURKISH, Language::GERMAN,
    Language::FRENCH, Language::ITALIAN, Language::SPANISH,
    Language::RUSSIAN
};

const QHash<QString, Language> LanguageManager::localeToLanguageMap = {
    {"en_US", Language::ENGLISH}, {"tr_TR", Language::TURKISH},
    {"de_DE", Language::GERMAN},  {"fr_FR", Language::FRENCH},
    {"it_IT", Language::ITALIAN}, {"es_ES", Language::SPANISH},
    {"ru_RU", Language::RUSSIAN}};


QString LanguageManager::languageToLocale(Language lang) {
  return languageToLocaleMap.value(lang, SettingsManager::defaultLanguageStr);
}

Language LanguageManager::localeToLanguage(const QString &locale) {
  return localeToLanguageMap.value(locale, SettingsManager::defaultLanguage);
}

bool LanguageManager::changeLanguage(const QString &languageCode) {
  QApplication::instance()->removeTranslator(&m_translator);

  bool loaded = m_translator.load(":/assets/locale/" + languageCode + ".qm");
  if (loaded) {
    QApplication::instance()->installTranslator(&m_translator);
    m_currentLocale = languageCode;
    emit languageChanged();
    lninfo << "Language changed to " + languageCode;
  } else {
    lnerr << "Failed to load language file: " + languageCode;
  }

  return loaded;
}

bool LanguageManager::changeLanguage(const Language &lang) {
  return changeLanguage(languageToLocale(lang));
}

bool LanguageManager::loadLanguage(const Language &lang) {
  return loadLanguage(languageToLocale(lang));
}

QString LanguageManager::getsyslang() {
  return QLocale::system().name();
}

bool LanguageManager::loadLanguage(const QString &languageCode) {
  QString selectedLocale = languageCode;

  if (selectedLocale.isEmpty()) {
    const QString sysloc = getsyslang();
    lninfo << "System locale: " + sysloc;

    QString qmPath = QString(":/assets/locale/%1.qm").arg(sysloc);
    selectedLocale = QFile::exists(qmPath) ? sysloc : SettingsManager::defaultLanguageStr;
  }

  return changeLanguage(selectedLocale);
}

QString LanguageManager::getCurrentLanguageStr() {
  return m_currentLocale.isEmpty() ? SettingsManager::defaultLanguageStr : m_currentLocale;
}

Language LanguageManager::getCurrentLanguage() {
  QString current = getCurrentLanguageStr();
  Language lang = localeToLanguage(current);

  return lang;
}
