#include "cryptomanager.h"
#include <QCryptographicHash>
#include <QString>
#include <QByteArray>
#include <QRandomGenerator>

// OpenSSL includes
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>

#include "../skeleton/loggerstream.h"

// Static member definitions
QByteArray CryptoManager::m_aesKey;
QByteArray CryptoManager::m_aesIV;
QString CryptoManager::m_rsaPublicKey;
QString CryptoManager::m_rsaPrivateKey;
QString CryptoManager::m_lastError;

// Global initialization
void CryptoManager::initializeCryptoManager() {
  OpenSSL_add_all_algorithms();
  ERR_load_crypto_strings();

  // Generate default keys
  CryptoManager::generateAESKey("DEFAULT_SEED");
  CryptoManager::generateRSAKeyPair(2048);

  cminfo << "CryptoManager initialized with default keys";
}

void CryptoManager::cleanupCryptoManager() {
  EVP_cleanup();
  ERR_free_strings();
  cminfo << "CryptoManager cleaned up";
}

CryptoManager::CryptoManager() {}

// Base64 functions (unchanged)
QString CryptoManager::b64encode(const QString& plaintxt) {
  return QString(plaintxt.toUtf8().toBase64());
}

QString CryptoManager::b64decode(const QString& code) {
  QByteArray decoded = QByteArray::fromBase64(code.toUtf8());
  return QString::fromUtf8(decoded);
}

// Error handling
QString CryptoManager::getLastError() {
  return m_lastError;
}

void CryptoManager::setError(const QString& error) {
  m_lastError = error;
  cmerr << "CryptoManager Error: " << error;
}

QString CryptoManager::extractOpenSSLError() {
  BIO* bio = BIO_new(BIO_s_mem());
  ERR_print_errors(bio);
  char* buf;
  size_t len = BIO_get_mem_data(bio, &buf);
  QString result = QString::fromUtf8(buf, len);
  BIO_free(bio);
  return result;
}

// Key management functions
void CryptoManager::generateAESKey(const QString& seed) {
  if (seed.isEmpty()) {
    // Generate random key and IV
    m_aesKey.resize(32); // 256-bit key
    m_aesIV.resize(16);  // 128-bit IV

    if (RAND_bytes(reinterpret_cast<unsigned char*>(m_aesKey.data()), 32) != 1) {
      // Fallback to Qt random if OpenSSL fails
      auto* generator = QRandomGenerator::global();
      for (int i = 0; i < 32; ++i) {
        m_aesKey[i] = static_cast<char>(generator->bounded(256));
      }
    }

    if (RAND_bytes(reinterpret_cast<unsigned char*>(m_aesIV.data()), 16) != 1) {
      // Fallback to Qt random if OpenSSL fails
      auto* generator = QRandomGenerator::global();
      for (int i = 0; i < 16; ++i) {
        m_aesIV[i] = static_cast<char>(generator->bounded(256));
      }
    }
  } else {
    // Generate key from seed
    QByteArray seedBytes = seed.toUtf8();

    // Use SHA-256 for key
    QByteArray keyHash = QCryptographicHash::hash(seedBytes, QCryptographicHash::Sha256);
    m_aesKey = keyHash; // SHA-256 = 32 bytes

    // Use SHA-256 of seed+salt for IV
    QByteArray ivSeed = seedBytes + "_iv_salt";
    QByteArray ivHash = QCryptographicHash::hash(ivSeed, QCryptographicHash::Sha256);
    m_aesIV = ivHash.left(16); // Take first 16 bytes
  }
}

void CryptoManager::generateRSAKeyPair(int keySize) {
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
  if (!ctx) {
    setError("Failed to create RSA key context");
    return;
  }

  if (EVP_PKEY_keygen_init(ctx) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    setError("Failed to initialize RSA key generation");
    return;
  }

  if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keySize) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    setError("Failed to set RSA key size");
    return;
  }

  EVP_PKEY* pkey = nullptr;
  if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
    EVP_PKEY_CTX_free(ctx);
    setError("Failed to generate RSA key pair");
    return;
  }

  EVP_PKEY_CTX_free(ctx);

  // Extract public key
  BIO* pubBio = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(pubBio, pkey);
  char* pubData;
  long pubLen = BIO_get_mem_data(pubBio, &pubData);
  m_rsaPublicKey = QString::fromUtf8(pubData, pubLen);
  BIO_free(pubBio);

  // Extract private key
  BIO* privBio = BIO_new(BIO_s_mem());
  PEM_write_bio_PrivateKey(privBio, pkey, nullptr, nullptr, 0, nullptr, nullptr);
  char* privData;
  long privLen = BIO_get_mem_data(privBio, &privData);
  m_rsaPrivateKey = QString::fromUtf8(privData, privLen);
  BIO_free(privBio);

  EVP_PKEY_free(pkey);
}

QString CryptoManager::getPublicKey() {
  return m_rsaPublicKey;
}

QString CryptoManager::getPrivateKey() {
  return m_rsaPrivateKey;
}

void CryptoManager::setAESKey(const QString& key, const QString& iv) {
  m_aesKey = QByteArray::fromHex(key.toUtf8());
  m_aesIV = QByteArray::fromHex(iv.toUtf8());

  if (m_aesKey.size() != 32) {
    setError("AES key must be 32 bytes (64 hex characters)");
    return;
  }
  if (m_aesIV.size() != 16) {
    setError("AES IV must be 16 bytes (32 hex characters)");
    return;
  }
}

void CryptoManager::setRSAKeys(const QString& publicKey, const QString& privateKey) {
  m_rsaPublicKey = publicKey;
  m_rsaPrivateKey = privateKey;
}

// AES encryption/decryption
QString CryptoManager::aesencrypt(const QString& txt) {
  if (m_aesKey.isEmpty() || m_aesIV.isEmpty()) {
    setError("AES keys not initialized");
    return QString();
  }

  QByteArray plaintext = txt.toUtf8();

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    setError("Failed to create cipher context");
    return QString();
  }

  // Initialize encryption
  if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const unsigned char*>(m_aesKey.constData()),
                         reinterpret_cast<const unsigned char*>(m_aesIV.constData())) != 1) {
    setError("Failed to initialize AES encryption");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }

  QByteArray ciphertext(plaintext.size() + AES_BLOCK_SIZE, 0);
  int len;
  int ciphertext_len;

  // Encrypt
  if (EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), &len,
                        reinterpret_cast<const unsigned char*>(plaintext.constData()),
                        plaintext.size()) != 1) {
    setError("Failed to encrypt data");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }
  ciphertext_len = len;

  // Finalize
  if (EVP_EncryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()) + len, &len) != 1) {
    setError("Failed to finalize encryption");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }
  ciphertext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  ciphertext.resize(ciphertext_len);
  return ciphertext.toBase64();
}

QString CryptoManager::aesdecrypt(const QString& crypted) {
  if (m_aesKey.isEmpty() || m_aesIV.isEmpty()) {
    setError("AES keys not initialized");
    return QString();
  }

  QByteArray ciphertext = QByteArray::fromBase64(crypted.toUtf8());

  EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
  if (!ctx) {
    setError("Failed to create cipher context");
    return QString();
  }

  // Initialize decryption
  if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const unsigned char*>(m_aesKey.constData()),
                         reinterpret_cast<const unsigned char*>(m_aesIV.constData())) != 1) {
    setError("Failed to initialize AES decryption");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }

  QByteArray plaintext(ciphertext.size(), 0);
  int len;
  int plaintext_len;

  // Decrypt
  if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(plaintext.data()), &len,
                        reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                        ciphertext.size()) != 1) {
    setError("Failed to decrypt data");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }
  plaintext_len = len;

  // Finalize
  if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(plaintext.data()) + len, &len) != 1) {
    setError("Failed to finalize decryption");
    EVP_CIPHER_CTX_free(ctx);
    return QString();
  }
  plaintext_len += len;

  EVP_CIPHER_CTX_free(ctx);

  plaintext.resize(plaintext_len);
  return QString::fromUtf8(plaintext);
}

// Helper function for RSA key loading
static EVP_PKEY* loadPublicKey(const QString& publicKey) {
  QByteArray keyBytes = publicKey.toUtf8();
  BIO* bio = BIO_new_mem_buf(keyBytes.constData(), keyBytes.length());
  if (!bio) return nullptr;

  EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);
  return pkey;
}

static EVP_PKEY* loadPrivateKey(const QString& privateKey) {
  QByteArray keyBytes = privateKey.toUtf8();
  BIO* bio = BIO_new_mem_buf(keyBytes.constData(), keyBytes.length());
  if (!bio) return nullptr;

  EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
  BIO_free(bio);
  return pkey;
}

// RSA encryption/decryption
QString CryptoManager::rsaencrypt(const QString& txt) {
  if (m_rsaPublicKey.isEmpty()) {
    setError("RSA public key not initialized");
    return QString();
  }

  QByteArray plaintext = txt.toUtf8();

  EVP_PKEY* pkey = loadPublicKey(m_rsaPublicKey);
  if (!pkey) {
    setError("Failed to load RSA public key");
    return QString();
  }

  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
  if (!ctx) {
    setError("Failed to create RSA encryption context");
    EVP_PKEY_free(pkey);
    return QString();
  }

  if (EVP_PKEY_encrypt_init(ctx) <= 0) {
    setError("Failed to initialize RSA encryption");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  size_t outlen;
  if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                       reinterpret_cast<const unsigned char*>(plaintext.constData()),
                       plaintext.size()) <= 0) {
    setError("Failed to determine RSA ciphertext length");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  QByteArray ciphertext(outlen, 0);
  if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(ciphertext.data()), &outlen,
                       reinterpret_cast<const unsigned char*>(plaintext.constData()),
                       plaintext.size()) <= 0) {
    setError("Failed to encrypt with RSA");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  ciphertext.resize(outlen);

  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(pkey);

  return ciphertext.toBase64();
}

QString CryptoManager::rsadecrypt(const QString& crypted) {
  if (m_rsaPrivateKey.isEmpty()) {
    setError("RSA private key not initialized");
    return QString();
  }

  QByteArray ciphertext = QByteArray::fromBase64(crypted.toUtf8());

  EVP_PKEY* pkey = loadPrivateKey(m_rsaPrivateKey);
  if (!pkey) {
    setError("Failed to load RSA private key");
    return QString();
  }

  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, nullptr);
  if (!ctx) {
    setError("Failed to create RSA decryption context");
    EVP_PKEY_free(pkey);
    return QString();
  }

  if (EVP_PKEY_decrypt_init(ctx) <= 0) {
    setError("Failed to initialize RSA decryption");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  size_t outlen;
  if (EVP_PKEY_decrypt(ctx, nullptr, &outlen,
                       reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                       ciphertext.size()) <= 0) {
    setError("Failed to determine RSA plaintext length");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  QByteArray plaintext(outlen, 0);
  if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(plaintext.data()), &outlen,
                       reinterpret_cast<const unsigned char*>(ciphertext.constData()),
                       ciphertext.size()) <= 0) {
    setError("Failed to decrypt with RSA");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return QString();
  }

  plaintext.resize(outlen);

  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(pkey);

  return QString::fromUtf8(plaintext);
}

// Hash functions (unchanged)
QString CryptoManager::md5hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Md5);
  return QString(hash.toHex());
}

QString CryptoManager::sha1hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Sha1);
  return QString(hash.toHex());
}

QString CryptoManager::sha224hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Sha224);
  return QString(hash.toHex());
}

QString CryptoManager::sha256hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Sha256);
  return QString(hash.toHex());
}

QString CryptoManager::sha384hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Sha384);
  return QString(hash.toHex());
}

QString CryptoManager::sha512hash(const QString& plaintxt) {
  QByteArray hash = QCryptographicHash::hash(plaintxt.toUtf8(), QCryptographicHash::Sha512);
  return QString(hash.toHex());
}
