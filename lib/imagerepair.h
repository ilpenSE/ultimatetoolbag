#ifndef IMAGEREPAIR_H
#define IMAGEREPAIR_H

#include <QByteArray>
#include <QFile>
#include <QIODevice>
#include <QString>

class ImageRepair {
 public:
  enum RepairResult {
    Success,
    FileNotFound,
    UnsupportedFormat,
    CorruptionTooSevere,
    ReadError,
    WriteError,
    UnknownError
  };

  enum ImageFormat { JPEG, PNG, Unknown };

  ImageRepair();
  ~ImageRepair();

  // main function
  RepairResult repairImage(const QString& inputPath,
                           const QString& outputPath = QString());
  ImageFormat detectFormat(const QByteArray& data);

  QString getLastError() const;
  QString getRepairReport() const;

 private:
  // JPEG repair
  RepairResult repairJPEG(QByteArray& data);
  bool fixJPEGHeader(QByteArray& data);
  bool fixJPEGFooter(QByteArray& data);
  bool validateJPEGMarkers(QByteArray& data);

  // PNG repair
  RepairResult repairPNG(QByteArray& data);
  bool fixPNGHeader(QByteArray& data);
  bool fixPNGFooter(QByteArray& data);
  bool validatePNGChunks(QByteArray& data);
  quint32 calculateCRC32(const QByteArray& data);

  // helpers
  bool writeToFile(const QString& path, const QByteArray& data);
  QByteArray readFile(const QString& path);
  void logRepair(const QString& message);

  // JPEG consts
  static const QByteArray JPEG_SOI;  // Start of Image
  static const QByteArray JPEG_EOI;  // End of Image

  // PNG consts
  static const QByteArray PNG_SIGNATURE;
  static const QByteArray PNG_IEND;

  QString m_lastError;
  QStringList m_repairLog;
};

#endif  // IMAGEREPAIR_H
