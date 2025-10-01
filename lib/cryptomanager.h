#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <QString>
#include <QByteArray>

class CryptoManager {
 public:
  CryptoManager();

  static void initializeCryptoManager();
  static void cleanupCryptoManager();

  // Base64 encoding/decoding
  static QString b64encode(const QString& plaintxt);
  static QString b64decode(const QString& code);

  // AES encryption/decryption
  static QString aesencrypt(const QString& txt);
  static QString aesdecrypt(const QString& crypted);

  // RSA encryption/decryption
  static QString rsaencrypt(const QString& txt);
  static QString rsadecrypt(const QString& crypted);

  // Hash functions
  static QString md5hash(const QString& plaintxt);
  static QString sha1hash(const QString& plaintxt);
  static QString sha224hash(const QString& plaintxt);
  static QString sha256hash(const QString& plaintxt);
  static QString sha384hash(const QString& plaintxt);
  static QString sha512hash(const QString& plaintxt);

  // Key management functions
  static void generateAESKey(const QString& seed = "");
  static void generateRSAKeyPair(int keySize = 2048);
  static QString getPublicKey();
  static QString getPrivateKey();

  // Set custom keys
  static void setAESKey(const QString& key, const QString& iv);
  static void setRSAKeys(const QString& publicKey, const QString& privateKey);

 private:
  // Internal storage for keys
  static QByteArray m_aesKey;
  static QByteArray m_aesIV;
  static QString m_rsaPublicKey;
  static QString m_rsaPrivateKey;

  // Helper functions
  static QString getLastError();
  static void setError(const QString& error);
  static QString extractOpenSSLError();

  static QString m_lastError;
};

#endif // CRYPTOMANAGER_H
