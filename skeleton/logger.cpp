#include "logger.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QTextStream>

#include "fileorganizer.h"
#include "../etc/instances.h"

bool Logger::s_alive = false;

Logger& Logger::instance() {
  static Logger _instance;
  return _instance;
}

Logger::Logger(QObject* parent) : QObject(parent) {
  s_alive = true;

  // AppData/logs klasörü yolu
  QString logsDirPath = _forg.appFolderPath() + "/logs";

  // Klasör yoksa oluştur
  QDir dir;
  if (!dir.exists(logsDirPath)) {
    if (!dir.mkpath(logsDirPath)) {
      qWarning() << "[LOGGER/ERROR] Cannot create log folder: " << logsDirPath;
    }
  }

  // Dosya adı
  QString fileName =
      QDateTime::currentDateTime().toString("dd.MM.yyyy-hh.mm.ss") + ".log";
  QString filePath = logsDirPath + "/" + fileName;

  m_logFile.setFileName(filePath);

  if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
    qWarning() << "[LOGGER/ERROR] Log file cannot be opened: " << filePath;
  }
}

Logger::~Logger() {
  s_alive = false;
  if (m_logFile.isOpen()) m_logFile.close();
}

bool Logger::isAlive() { return s_alive; }

// theme logs
void Logger::thInfo(const QString& message) { log("[THEMES/INFO]", message); }

void Logger::thWarning(const QString& message) { log("[THEMES/WARNING]", message); }

void Logger::thError(const QString& message) { log("[THEMES/ERROR]", message); }

void Logger::thCrit(const QString& message) { log("[THEMES/FATAL]", message); }

// settings logs
void Logger::sInfo(const QString& message) { log("[SETTINGS/INFO]", message); }

void Logger::sWarning(const QString& message) { log("[SETTINGS/WARNING]", message); }

void Logger::sError(const QString& message) { log("[SETTINGS/ERROR]", message); }

void Logger::sCrit(const QString& message) { log("[SETTINGS/FATAL]", message); }

// file system logs
void Logger::fsInfo(const QString& message) { log("[FS/INFO]", message); }

void Logger::fsWarning(const QString& message) { log("[FS/WARNING]", message); }

void Logger::fsError(const QString& message) { log("[FS/ERROR]", message); }

void Logger::fsCrit(const QString& message) { log("[FS/FATAL]", message); }

// language logs
void Logger::langInfo(const QString& message) { log("[LANG/INFO]", message); }

void Logger::langWarning(const QString& message) { log("[LANG/WARNING]", message); }

void Logger::langError(const QString& message) { log("[LANG/ERROR]", message); }

void Logger::langCrit(const QString& message) { log("[LANG/FATAL]", message); }

// default logs
void Logger::debugInfo(const QString& message) { log("[DEBUG]", message); }

void Logger::logInfo(const QString& message) { log("[INFO]", message); }

void Logger::logWarning(const QString& message) { log("[WARNING]", message); }

void Logger::logError(const QString& message) { log("[ERROR]", message); }

void Logger::logCrit(const QString& message) { log("[FATAL ERROR]", message); }

// image repair logs
void Logger::irInfo(const QString& message) { log("[IMAGE REPAIR/INFO]", message); }

void Logger::irWarning(const QString& message) { log("[IMAGE REPAIR/WARNING]", message); }

void Logger::irError(const QString& message) { log("[IMAGE REPAIR/ERROR]", message); }

void Logger::irCrit(const QString& message) { log("[IMAGE REPAIR/FATAL]", message); }

// json logs
void Logger::jsonError(const QString& message) { log("[JSON WORKER/ERROR]", message); }
void Logger::jsonWarning(const QString& message) { log("[JSON WORKER/WARNING]", message); }

// crypto logs
void Logger::cryptoInfo(const QString& message) { log("[CRYPTOGRAPHY/INFO]", message); }

void Logger::cryptoWarning(const QString& message) { log("[CRYPTOGRAPHY/WARNING]", message); }

void Logger::cryptoError(const QString& message) { log("[CRYPTOGRAPHY/ERROR]", message); }

void Logger::log(const QString& level, const QString& message) {
  QString timeStamp =
      QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
  QString logLine = QString("%1 %2 %3").arg(timeStamp, level, message);

  qDebug() << logLine;

  if (m_logFile.isOpen()) {
    QTextStream out(&m_logFile);
    out << logLine << "\n";
    out.flush();
  }
}
