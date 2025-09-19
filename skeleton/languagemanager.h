#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QHash>
#include <QObject>
#include <QTranslator>

#include "../etc/enums.h"

class LanguageManager : public QObject {
  Q_OBJECT
 public:
  static LanguageManager &instance() {
    static LanguageManager _instance;
    return _instance;
  }

  static const QHash<Language, QString> languageToLocaleMap;
  static const QHash<QString, Language> localeToLanguageMap;

  bool changeLanguage(const QString &languageCode);
  bool changeLanguage(const Language &language);

  bool loadLanguage(const QString &languageCode = QString());
  bool loadLanguage(const Language &lang);

  static QString getsyslang();
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
  QString m_currentLocale;

  // block copy and assign
  LanguageManager(const LanguageManager &) = delete;
  LanguageManager &operator=(const LanguageManager &) = delete;

  ~LanguageManager() override = default;
};

#endif  // LANGUAGEMANAGER_H
