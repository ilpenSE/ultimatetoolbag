#include "imagerepair.h"

#include <QCryptographicHash>

#include "../skeleton/loggerstream.h"

// JPEG consts
const QByteArray ImageRepair::JPEG_SOI = QByteArray::fromHex("FFD8");
const QByteArray ImageRepair::JPEG_EOI = QByteArray::fromHex("FFD9");

// PNG consts
const QByteArray ImageRepair::PNG_SIGNATURE =
    QByteArray::fromHex("89504E470D0A1A0A");
const QByteArray ImageRepair::PNG_IEND =
    QByteArray::fromHex("0000000049454E44AE426082");

ImageRepair::ImageRepair() {
  m_lastError.clear();
  m_repairLog.clear();
}

ImageRepair::~ImageRepair() {}

ImageRepair::RepairResult ImageRepair::repairImage(const QString& inputPath,
                                                   const QString& outputPath) {
  m_lastError.clear();
  m_repairLog.clear();

  // read file
  QByteArray imageData = readFile(inputPath);
  if (imageData.isEmpty()) {
    m_lastError = "File couldn't be read: " + inputPath;
    return FileNotFound;
  }

  logRepair(QString("File read: %1 bytes").arg(imageData.size()));

  // detect format
  ImageFormat format = detectFormat(imageData);
  RepairResult result = UnknownError;
  switch (format) {
    case JPEG:
      logRepair("JPEG format detected");
      result = repairJPEG(imageData);
      break;

    case PNG:
      logRepair("PNG format detected");
      result = repairPNG(imageData);
      break;

    default:
      m_lastError = "Unsupported file format";
      return UnsupportedFormat;
  }

  // save file if success
  if (result == Success) {
    QString savePath = outputPath.isEmpty()
                           ? inputPath.left(inputPath.lastIndexOf('.')) +
                                 "_repaired" +
                                 inputPath.mid(inputPath.lastIndexOf('.'))
                           : outputPath;

    if (writeToFile(savePath, imageData)) {
      logRepair("Repaired file saved to " + savePath);
    } else {
      m_lastError = "Cannot write file " + savePath;
      return WriteError;
    }
  }

  return result;
}

ImageRepair::ImageFormat ImageRepair::detectFormat(const QByteArray& data) {
  if (data.size() < 8) return Unknown;

  // JPEG control (starts with FF D8)
  if (data.startsWith(JPEG_SOI)) {
    return JPEG;
  }

  // PNG control (starts with 89 50 4E 47 0D 0A 1A 0A)
  if (data.startsWith(PNG_SIGNATURE)) {
    return PNG;
  }

  // if header corrupted, look at the content and guess
  if (data.contains(QByteArrayLiteral("JFIF")) ||
      data.contains(QByteArrayLiteral("Exif")))
    return JPEG;
  if (data.contains(QByteArrayLiteral("IHDR")) ||
      data.contains(QByteArrayLiteral("PLTE")))
    return PNG;

  return Unknown;
}

ImageRepair::RepairResult ImageRepair::repairJPEG(QByteArray& data) {
  bool repaired = false;

  // Header control
  if (!data.startsWith(JPEG_SOI)) {
    logRepair("JPEG header corrupted, repairing...");
    if (fixJPEGHeader(data)) {
      repaired = true;
      logRepair("JPEG header repaired");
    }
  }

  // Footer control
  if (!data.endsWith(JPEG_EOI)) {
    logRepair("JPEG footer corrupted, repairing...");
    if (fixJPEGFooter(data)) {
      repaired = true;
      logRepair("JPEG footer repaired");
    }
  }

  // Marker validation
  if (validateJPEGMarkers(data)) {
    if (!repaired) {
      logRepair("JPEG file already good");
    }
    return Success;
  }

  m_lastError = "JPEG corrupted too severe";
  return CorruptionTooSevere;
}

bool ImageRepair::fixJPEGHeader(QByteArray& data) {
  // find real header with JFIF or Exif
  int jfifPos = data.indexOf(QByteArrayLiteral("JFIF"));
  int exifPos = data.indexOf(QByteArrayLiteral("Exif"));

  int startPos = -1;
  if (jfifPos != -1)
    startPos = jfifPos;
  else if (exifPos != -1)
    startPos = exifPos;

  if (startPos > 0) {
    int markerPos = startPos - 2;
    if (markerPos > 0) {
      data = JPEG_SOI + data.mid(markerPos);
      return true;
    }
  }

  // repair by finding another marker
  for (int i = 0; i < data.size() - 1; i++) {
    if (data[i] == '\xFF' && (quint8)data[i + 1] >= 0xC0) {
      data = JPEG_SOI + data.mid(i);
      return true;
    }
  }

  return false;
}

bool ImageRepair::fixJPEGFooter(QByteArray& data) {
  for (int i = data.size() - 2; i >= 0; i--) {
    if (data[i] == '\xFF' && data[i + 1] != '\x00') {
      data = data.left(i + 2) + JPEG_EOI;
      return true;
    }
  }

  data.append(JPEG_EOI);
  return true;
}

bool ImageRepair::validateJPEGMarkers(QByteArray& data) {
  if (data.size() < 4) return false;

  if (!data.startsWith(JPEG_SOI)) return false;
  if (!data.endsWith(JPEG_EOI)) return false;

  // control the fundamental marker structure
  bool hasValidStructure = false;
  for (int i = 2; i < data.size() - 2; i++) {
    if (data[i] == '\xFF') {
      quint8 marker = (quint8)data[i + 1];
      if (marker >= 0xC0 && marker <= 0xFE && marker != 0xFF) {
        hasValidStructure = true;
        break;
      }
    }
  }

  return hasValidStructure;
}

ImageRepair::RepairResult ImageRepair::repairPNG(QByteArray& data) {
  bool repaired = false;

  // header control
  if (!data.startsWith(PNG_SIGNATURE)) {
    logRepair("PNG signature corrupted, repairing...");
    if (fixPNGHeader(data)) {
      repaired = true;
      logRepair("PNG signature repaired");
    }
  }

  // Footer control
  if (!data.endsWith(PNG_IEND)) {
    logRepair("PNG IEND chunk corrupted, repairing...");
    if (fixPNGFooter(data)) {
      repaired = true;
      logRepair("PNG IEND chunk repaired");
    }
  }

  // Chunk validation
  if (validatePNGChunks(data)) {
    if (!repaired) {
      logRepair("Png file already good");
    }
    return Success;
  }

  m_lastError = "PNG file corrupted too severe";
  return CorruptionTooSevere;
}

bool ImageRepair::fixPNGHeader(QByteArray& data) {
  // find real header with IHDR
  int ihdrPos = data.indexOf(QByteArrayLiteral("IHDR"));
  if (ihdrPos >= 4) {
    data = PNG_SIGNATURE + data.mid(ihdrPos - 4);
    return true;
  }

  // Repair PNG text chunks by finding'em
  QList<QByteArray> pngChunks = {
      QByteArrayLiteral("PLTE"), QByteArrayLiteral("tRNS"),
      QByteArrayLiteral("gAMA"), QByteArrayLiteral("cHRM"),
      QByteArrayLiteral("sRGB"), QByteArrayLiteral("tEXt")};
  for (const QByteArray& chunk : pngChunks) {
    int pos = data.indexOf(chunk);
    if (pos >= 4) {
      data = PNG_SIGNATURE + data.mid(pos - 4);
      return true;
    }
  }

  return false;
}

bool ImageRepair::fixPNGFooter(QByteArray& data) {
  QList<QByteArray> pngChunks = {
      QByteArrayLiteral("IHDR"), QByteArrayLiteral("PLTE"),
      QByteArrayLiteral("IDAT"), QByteArrayLiteral("tRNS"),
      QByteArrayLiteral("gAMA"), QByteArrayLiteral("cHRM"),
      QByteArrayLiteral("sRGB"), QByteArrayLiteral("tEXt")};

  int lastValidPos = -1;
  for (const QByteArray& chunk : pngChunks) {
    int pos = data.lastIndexOf(chunk);
    if (pos > lastValidPos) {
      lastValidPos = pos;
    }
  }

  if (lastValidPos > 0) {
    // guess the chunk size (usually there's CRC after the chunk name)
    int endPos = lastValidPos + 4 + 4;  // chunk name + CRC
    if (endPos < data.size()) {
      data = data.left(endPos) + PNG_IEND;
      return true;
    }
  }

  data.append(PNG_IEND);
  return true;
}

bool ImageRepair::validatePNGChunks(QByteArray& data) {
  if (data.size() < 33) return false;  // En az signature + IHDR + IEND

  if (!data.startsWith(PNG_SIGNATURE)) return false;

  if (data.indexOf(QByteArrayLiteral("IHDR"), 8) == -1) return false;

  if (!data.endsWith(PNG_IEND)) return false;

  return true;
}

quint32 ImageRepair::calculateCRC32(const QByteArray& data) {
  // Basic CRC32 calculation (enough for PNG)
  quint32 crc = 0xFFFFFFFF;
  for (int i = 0; i < data.size(); ++i) {
    quint8 byte = static_cast<quint8>(data[i]);
    crc ^= byte;
    for (int j = 0; j < 8; ++j) {
      if (crc & 1) {
        crc = (crc >> 1) ^ 0xEDB88320;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc ^ 0xFFFFFFFF;
}

QByteArray ImageRepair::readFile(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    return QByteArray();
  }
  return file.readAll();
}

bool ImageRepair::writeToFile(const QString& path, const QByteArray& data) {
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }
  return file.write(data) == data.size();
}

void ImageRepair::logRepair(const QString& message) {
  m_repairLog.append(message);
  irinfo << message;
}

QString ImageRepair::getLastError() const { return m_lastError; }

QString ImageRepair::getRepairReport() const { return m_repairLog.join("\n"); }
