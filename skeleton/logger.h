#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QObject>

// stream için makrolar
// normal log
#define ldebug LoggerStream(&Logger::debugInfo)
#define linfo LoggerStream(&Logger::logInfo)
#define warn LoggerStream(&Logger::logWarning)
#define err LoggerStream(&Logger::logError)
#define crit LoggerStream(&Logger::logCrit)

// settings
#define sinfo LoggerStream(&Logger::sInfo)
#define swrn LoggerStream(&Logger::sWarning)
#define serr LoggerStream(&Logger::sError)
#define scrit LoggerStream(&Logger::sCrit)

// file system
#define fsinfo LoggerStream(&Logger::fsInfo)
#define fswrn LoggerStream(&Logger::fsWarning)
#define fserr LoggerStream(&Logger::fsError)
#define fscrit LoggerStream(&Logger::fsCrit)

// themes
#define thinfo LoggerStream(&Logger::thInfo)
#define thwrn LoggerStream(&Logger::thWarning)
#define therr LoggerStream(&Logger::thError)
#define thcrit LoggerStream(&Logger::thCrit)

// language
#define lninfo LoggerStream(&Logger::langInfo)
#define lnwrn LoggerStream(&Logger::langWarning)
#define lnerr LoggerStream(&Logger::langError)
#define lncrit LoggerStream(&Logger::langCrit)

// imagerepair
#define irinfo LoggerStream(&Logger::irInfo)
#define irwrn LoggerStream(&Logger::irWarning)
#define irerr LoggerStream(&Logger::irError)
#define ircrit LoggerStream(&Logger::irCrit)

// json worker
#define jserr LoggerStream(&Logger::jsonError)
#define jswrn LoggerStream(&Logger::jsonWarning)

// crypto manager
#define cmerr LoggerStream(&Logger::cryptoError)
#define cmwarn LoggerStream(&Logger::cryptoWarning)
#define cminfo LoggerStream(&Logger::cryptoInfo)

class Logger : public QObject {
  Q_OBJECT
 public:
  static Logger& instance();
  static bool isAlive();

  // default log
  void debugInfo(const QString& message);
  void logInfo(const QString& message);
  void logWarning(const QString& message);
  void logError(const QString& message);
  void logCrit(const QString& message);

  // fs log
  void fsInfo(const QString& message);
  void fsWarning(const QString& message);
  void fsError(const QString& message);
  void fsCrit(const QString& message);

  // lang log
  void langInfo(const QString& message);
  void langWarning(const QString& message);
  void langError(const QString& message);
  void langCrit(const QString& message);

  // settings log
  void sInfo(const QString& message);
  void sWarning(const QString& message);
  void sError(const QString& message);
  void sCrit(const QString& message);

  // themes log
  void thInfo(const QString& message);
  void thWarning(const QString& message);
  void thError(const QString& message);
  void thCrit(const QString& message);

  // image repair log
  void irInfo(const QString& message);
  void irWarning(const QString& message);
  void irError(const QString& message);
  void irCrit(const QString& message);

  // json log
  void jsonError(const QString& message);
  void jsonWarning(const QString& message);

  // crypto logs
  void cryptoError(const QString& message);
  void cryptoInfo(const QString& message);
  void cryptoWarning(const QString& message);

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
