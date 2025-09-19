#ifndef LOGGERSTREAM_H
#define LOGGERSTREAM_H

#include <QString>
#include <QTextStream>

#include "logger.h"

class LoggerStream {
 public:
  LoggerStream(void (Logger::*logFunc)(const QString&))
      : m_stream(&m_buffer, QIODevice::WriteOnly), m_logFunc(logFunc) {}

  ~LoggerStream() {
    if (!m_buffer.isEmpty() && Logger::isAlive()) {
      (Logger::instance().*m_logFunc)(m_buffer);
    }
  }

  template <typename T>
  LoggerStream& operator<<(const T& value) {
    m_stream << value;
    return *this;
  }

 private:
  QString m_buffer;
  QTextStream m_stream;
  void (Logger::*m_logFunc)(const QString&);
};

#endif  // LOGGERSTREAM_H
