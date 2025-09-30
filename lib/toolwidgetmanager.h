#ifndef TOOLWIDGETMANAGER_H
#define TOOLWIDGETMANAGER_H

#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include <functional>
#include <QStringList>

#include "../etc/enums.h"

class QWidget;
class QString;

class ToolWidgetManager : public QObject {
  Q_OBJECT
 public:
  // Meyers singleton
  static ToolWidgetManager& instance() {
    static ToolWidgetManager _instance;
    return _instance;
  }

  // Factory function type
  using ToolFactory = std::function<QWidget*()>;

  // Tool information structure
  struct ToolInfo {
    QString displayName;
    ToolCategory category;
    QString description;
    bool cacheable = true;
    bool singleton = false;
  };

 private:
  // Static factory registry
  static QHash<QString, ToolFactory> s_toolFactories;
  static QHash<QString, ToolInfo> s_toolInfo;
  static bool s_initialized;

  // Instance cache
  mutable QHash<QString, QSharedPointer<QWidget>> m_widgetCache;

 public:
  explicit ToolWidgetManager(QObject* parent = nullptr);
  ~ToolWidgetManager();

  QWidget* createWidget(const QString& toolName, bool forceNew = false);

  // Static widget creators - can be used directly
  // File Converters
  static QWidget* createPdfWordWidget();
  static QWidget* createZipRarWidget();
  static QWidget* createVideoAudioWidget();

  // Dev Tools
  static QWidget* createJsonPrettierWidget();
  static QWidget* createCurlWidget();
  static QWidget* createColorPickerWidget();
  static QWidget* createRegexTesterWidget();
  static QWidget* createApiTesterWidget();

  // Cryptors
  static QWidget* createHashGenWidget();
  static QWidget* createAesRsaWidget();
  static QWidget* createBaseEncoderWidget();

  // Unit Converters
  static QWidget* createLengthConverterWidget();
  static QWidget* createTimeConverterWidget();
  static QWidget* createWeightConverterWidget();
  static QWidget* createTemperatureConverterWidget();
  static QWidget* createCurrencyConverterWidget();

  // Fixers
  static QWidget* createImageFixerWidget();
  static QWidget* createJsonFixerWidget();

  // Utility methods
  QStringList getAvailableTools() const;
  QStringList getToolsByCategory(const QString& category) const;
  QStringList getCategories() const;
  ToolInfo getToolInfo(const QString& toolName) const;

  // Cache management
  void clearCache();
  void clearCache(const QString& toolName);
  int getCacheSize() const;

 private:
  // Initialize factory registry
  static void initializeFactories();

  // Helper methods for creating specific widget types
  static QWidget* createCrypterWidget(const QString& title, CryptorTools cryptorTool);
  static QWidget* createUnitConverterWidget(const QString& title,
                                            const QStringList& units,
                                            UnitConverterType converterType);

  // Unit name providers
  static QStringList getLengthUnitNames();
  static QStringList getTimeUnitNames();
  static QStringList getTemperatureUnitNames();
  static QStringList getWeightUnitNames();
  static QStringList getCurrencyUnitNames();
};

#endif // TOOLWIDGETMANAGER_H
