#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QObject>

// stream için makrolar
// normal log
#define info() LoggerStream(&Logger::logInfo)
#define warn() LoggerStream(&Logger::logWarning)
#define err() LoggerStream(&Logger::logError)

// settings
#define sinfo() LoggerStream(&Logger::sInfo)
#define swrn() LoggerStream(&Logger::sWarning)
#define serr() LoggerStream(&Logger::sError)

// file system
#define fsinfo() LoggerStream(&Logger::fsInfo)
#define fswrn() LoggerStream(&Logger::fsWarning)
#define fserr() LoggerStream(&Logger::fsError)

// macros
#define minfo() LoggerStream(&Logger::mInfo)
#define mwrn() LoggerStream(&Logger::mWarning)
#define merr() LoggerStream(&Logger::mError)

// themes
#define thinfo() LoggerStream(&Logger::thInfo)
#define thwrn() LoggerStream(&Logger::thWarning)
#define therr() LoggerStream(&Logger::thError)

// language
#define lninfo() LoggerStream(&Logger::langInfo)
#define lnwrn() LoggerStream(&Logger::langWarning)
#define lnerr() LoggerStream(&Logger::langError)

// hotkey service
#define hsinfo() LoggerStream(&Logger::hsInfo)
#define hswrn() LoggerStream(&Logger::hsWarning)
#define hserr() LoggerStream(&Logger::hsError)

// clicker engine
#define cinfo() LoggerStream(&Logger::cInfo)
#define cwrn() LoggerStream(&Logger::cWarning)
#define cerr() LoggerStream(&Logger::cError)

class Logger : public QObject {
  Q_OBJECT
 public:
  static Logger& instance();
  static bool isAlive();

  // default log
  void logInfo(const QString& message);
  void logWarning(const QString& message);
  void logError(const QString& message);

  // fs log
  void fsInfo(const QString& message);
  void fsWarning(const QString& message);
  void fsError(const QString& message);

  // lang log
  void langInfo(const QString& message);
  void langWarning(const QString& message);
  void langError(const QString& message);

  // settings log
  void sInfo(const QString& message);
  void sWarning(const QString& message);
  void sError(const QString& message);

  // themes log
  void thInfo(const QString& message);
  void thWarning(const QString& message);
  void thError(const QString& message);

  // macros log
  void mInfo(const QString& message);
  void mWarning(const QString& message);
  void mError(const QString& message);

  // hotkey service log
  void hsInfo(const QString& message);
  void hsWarning(const QString& message);
  void hsError(const QString& message);

  // clicker engine log
  void cInfo(const QString& message);
  void cWarning(const QString& message);
  void cError(const QString& message);

 private:
  explicit Logger(QObject* parent = nullptr);
  ~Logger() override;

  void log(const QString& level, const QString& message);

  static bool s_alive;

  QFile m_logFile;

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;
};

#endif  // LOGGER_H
