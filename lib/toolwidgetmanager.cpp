#include "toolwidgetmanager.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDebug>

#include "../etc/apptranslations.h"
#include "unitconverter.h"
#include "../skeleton/loggerstream.h"

// Static member definitions
QHash<QString, ToolWidgetManager::ToolFactory> ToolWidgetManager::s_toolFactories;
QHash<QString, ToolWidgetManager::ToolInfo> ToolWidgetManager::s_toolInfo;
bool ToolWidgetManager::s_initialized = false;

ToolWidgetManager::ToolWidgetManager(QObject* parent)
    : QObject(parent)
{
  if (!s_initialized) {
    initializeFactories();
    s_initialized = true;
  }
}

ToolWidgetManager::~ToolWidgetManager() {
  clearCache();
}

void ToolWidgetManager::initializeFactories() {
  // File Converter
  s_toolFactories["pdfword"] = []() { return createPdfWordWidget(); };
  s_toolInfo["pdfword"] = {"PDF Word Converter", ToolCategory::FILE_CONVERTERS, "Convert PDF between Word", true, false};

  s_toolFactories["ziprar"] = []() { return createZipRarWidget(); };
  s_toolInfo["ziprar"] = {"ZIP/RAR Manager", ToolCategory::FILE_CONVERTERS, "Compress and extract archives", true, false};

  s_toolFactories["videoaudio"] = []() { return createJsonPrettierWidget(); };
  s_toolInfo["videoaudio"] = {"Video Audio Converter", ToolCategory::FILE_CONVERTERS, "Convert video between audio", true, false};

  // Cryptors
  s_toolFactories["hashgen"] = []() { return createHashGenWidget(); };
  s_toolInfo["hashgen"] = {"Hash Generator", ToolCategory::CRYPTORS, "Generate various hashes", true, false};

  s_toolFactories["aesrsa"] = []() { return createAesRsaWidget(); };
  s_toolInfo["aesrsa"] = {"AES/RSA Cryptor", ToolCategory::CRYPTORS, "Encrypt/decrypt data", true, false};

  s_toolFactories["baseencoder"] = []() { return createBaseEncoderWidget(); };
  s_toolInfo["baseencoder"] = {"Base Encoder", ToolCategory::CRYPTORS, "Base64/32/16 encoding", true, false};

  // Developer Tools
  s_toolFactories["colorpicker"] = []() { return createColorPickerWidget(); };
  s_toolInfo["colorpicker"] = {"Color Picker", ToolCategory::DEV_TOOLS, "Pick and convert colors", true, true};

  s_toolFactories["regextester"] = []() { return createRegexTesterWidget(); };
  s_toolInfo["regextester"] = {"Regex Tester", ToolCategory::DEV_TOOLS, "Test regular expressions", true, false};

  s_toolFactories["curl"] = []() { return createCurlWidget(); };
  s_toolInfo["curl"] = {"CURL Tool", ToolCategory::DEV_TOOLS, "Make HTTP requests", true, false};

  s_toolFactories["apitester"] = []() { return createApiTesterWidget(); };
  s_toolInfo["apitester"] = {"API Tester", ToolCategory::DEV_TOOLS, "Test REST APIs", true, false};

  s_toolFactories["jsonprettier"] = []() { return createJsonPrettierWidget(); };
  s_toolInfo["jsonprettier"] = {"JSON Prettifier", ToolCategory::DEV_TOOLS, "Format and beautify JSON", true, false};

  // Unit Converters
  s_toolFactories["length"] = []() { return createLengthConverterWidget(); };
  s_toolInfo["length"] = {"Length Converter", ToolCategory::UNIT_CONVERTERS, "Convert length units", true, false};

  s_toolFactories["time"] = []() { return createTimeConverterWidget(); };
  s_toolInfo["time"] = {"Time Converter", ToolCategory::UNIT_CONVERTERS, "Convert time units", true, false};

  s_toolFactories["weight"] = []() { return createWeightConverterWidget(); };
  s_toolInfo["weight"] = {"Weight Converter", ToolCategory::UNIT_CONVERTERS, "Convert weight units", true, false};

  s_toolFactories["temperature"] = []() { return createTemperatureConverterWidget(); };
  s_toolInfo["temperature"] = {"Temperature Converter", ToolCategory::UNIT_CONVERTERS, "Convert temperature units", true, false};

  s_toolFactories["currency"] = []() { return createCurrencyConverterWidget(); };
  s_toolInfo["currency"] = {"Currency Converter", ToolCategory::UNIT_CONVERTERS, "Convert currencies", true, false};

  // Fixers
  s_toolFactories["imagefixer"] = []() { return createImageFixerWidget(); };
  s_toolInfo["imagefixer"] = {"Image Fixer", ToolCategory::FIXERS, "Fix and enhance images", true, false};

  s_toolFactories["jsonfixer"] = []() { return createJsonFixerWidget(); };
  s_toolInfo["jsonfixer"] = {"JSON Fixer", ToolCategory::FIXERS, "Fix malformed JSON", true, false};


  qDebug() << "ToolWidgetManager: Initialized" << s_toolFactories.size() << "tools";
}

QWidget* ToolWidgetManager::createWidget(const QString& toolName, bool forceNew) {
  if (!s_toolFactories.contains(toolName)) {
    qWarning() << "ToolWidgetManager: Unknown tool:" << toolName;
    return nullptr;
  }

  const ToolInfo& info = s_toolInfo[toolName];

  // Singleton check
  if (info.singleton && !forceNew && m_widgetCache.contains(toolName)) {
    auto cached = m_widgetCache[toolName];
    if (cached) {
      return cached.data();
    }
  }

  // Cache check
  if (info.cacheable && !forceNew && m_widgetCache.contains(toolName)) {
    auto cached = m_widgetCache[toolName];
    if (cached && !cached->isVisible()) {
      return cached.data();
    }
  }

  // Create new widget
  QWidget* newWidget = s_toolFactories[toolName]();
  if (!newWidget) {
    qWarning() << "ToolWidgetManager: Failed to create widget for:" << toolName;
    return nullptr;
  }

  // Add to cache if cacheable
  if (info.cacheable) {
    auto sharedWidget = QSharedPointer<QWidget>(newWidget);
    m_widgetCache[toolName] = sharedWidget;

    // Remove from cache when destroyed
    connect(newWidget, &QObject::destroyed, this, [this, toolName]() {
      m_widgetCache.remove(toolName);
    });
  }

  return newWidget;
}

// Static widget creators
// FILE CONVERTERS
QWidget* ToolWidgetManager::createPdfWordWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("PDF Word Converter - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createZipRarWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("ZIP/RAR Manager - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createVideoAudioWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("Video/Audio Manager - Coming Soon"));
  return widget;
}

// CRYPTORS
QWidget* ToolWidgetManager::createHashGenWidget() {
  return createCrypterWidget(_tr("hashgen"), CryptorTools::HASHER);
}

QWidget* ToolWidgetManager::createAesRsaWidget() {
  return createCrypterWidget(_tr("aesrsa"), CryptorTools::AESRSA);
}

QWidget* ToolWidgetManager::createBaseEncoderWidget() {
  return createCrypterWidget(_tr("baseencoder"), CryptorTools::BASECODER);
}

// DEV TOOLS
QWidget* ToolWidgetManager::createJsonPrettierWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("JSON Prettifier - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createCurlWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("CURL Tool - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createColorPickerWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("Color Picker - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createRegexTesterWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("Regex Tester - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createApiTesterWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("API Tester - Coming Soon"));
  return widget;
}

// UNIT CONVERTERS
QWidget* ToolWidgetManager::createLengthConverterWidget() {
  return createUnitConverterWidget(
      _tr("length"),
      getLengthUnitNames(),
      UnitConverterType::LENGTH);
}

QWidget* ToolWidgetManager::createTimeConverterWidget() {
  return createUnitConverterWidget(
      _tr("time"),
      getTimeUnitNames(),
      UnitConverterType::TIME);
}

QWidget* ToolWidgetManager::createWeightConverterWidget() {
  return createUnitConverterWidget(
      _tr("weight"),
      getWeightUnitNames(),
      UnitConverterType::WEIGHT);
}

QWidget* ToolWidgetManager::createTemperatureConverterWidget() {
  return createUnitConverterWidget(
      _tr("temperature"),
      getTemperatureUnitNames(),
      UnitConverterType::TEMPERATURE);
}

QWidget* ToolWidgetManager::createCurrencyConverterWidget() {
  return createUnitConverterWidget(
      _tr("currency") + " (NON-FUNCTIONAL)",
      getCurrencyUnitNames(),
      UnitConverterType::CURRENCY);
}

// FIXERS
QWidget* ToolWidgetManager::createImageFixerWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("Image Fixer - Coming Soon"));
  return widget;
}

QWidget* ToolWidgetManager::createJsonFixerWidget() {
  QWidget* widget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel("JSON Fixer - Coming Soon"));
  return widget;
}

// Helper methods
QWidget* ToolWidgetManager::createCrypterWidget(const QString& title, CryptorTools ctool) {
  QWidget* mainWidget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(mainWidget);

  QLabel* titleLabel = new QLabel(title);
  titleLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(titleLabel);

  // TODO: Implement crypter UI based on algorithm
  QLabel* placeholderLabel = new QLabel("Crypter widget - Coming Soon");
  placeholderLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(placeholderLabel);

  return mainWidget;
}

QWidget* ToolWidgetManager::createUnitConverterWidget(const QString& title,
                                                      const QStringList& units,
                                                      UnitConverterType converterType) {
  QWidget* mainWidget = new QWidget();
  QHBoxLayout* mainLayout = new QHBoxLayout(mainWidget);
  mainLayout->setSpacing(15);

  // --- Input layout ---
  QVBoxLayout* inputLayout = new QVBoxLayout;
  QLabel* fromLabel = new QLabel("Input");
  fromLabel->setAlignment(Qt::AlignCenter);

  QHBoxLayout* fromBoxLayout = new QHBoxLayout;
  QDoubleSpinBox* inputSpin = new QDoubleSpinBox;
  inputSpin->setRange(-1e9, 1e9);
  inputSpin->setDecimals(6);
  inputSpin->setValue(1.0);

  QComboBox* fromCombo = new QComboBox;
  fromCombo->addItems(units);

  fromBoxLayout->addWidget(inputSpin);
  fromBoxLayout->addWidget(fromCombo);

  inputLayout->addWidget(fromLabel);
  inputLayout->addLayout(fromBoxLayout);

  // --- Mid layout ---
  QVBoxLayout* middleLayout = new QVBoxLayout;
  QPushButton* swapBtn = new QPushButton("⇄");
  swapBtn->setToolTip("Swap units");

  QPushButton* convertBtn = new QPushButton("Convert");

  middleLayout->addWidget(swapBtn);
  middleLayout->addWidget(convertBtn);
  middleLayout->setAlignment(Qt::AlignCenter);

  // --- Output layout ---
  QVBoxLayout* outputLayout = new QVBoxLayout;
  QLabel* toLabel = new QLabel("Output");
  toLabel->setAlignment(Qt::AlignCenter);

  QHBoxLayout* toBoxLayout = new QHBoxLayout;
  QDoubleSpinBox* outputSpin = new QDoubleSpinBox;
  outputSpin->setRange(-1e9, 1e9);
  outputSpin->setDecimals(6);
  outputSpin->setReadOnly(true);

  QComboBox* toCombo = new QComboBox;
  toCombo->addItems(units);
  if (units.size() > 1) toCombo->setCurrentIndex(1);

  toBoxLayout->addWidget(outputSpin);
  toBoxLayout->addWidget(toCombo);

  outputLayout->addWidget(toLabel);
  outputLayout->addLayout(toBoxLayout);

  // --- Main layout addition ---
  mainLayout->addLayout(inputLayout);
  mainLayout->addLayout(middleLayout);
  mainLayout->addLayout(outputLayout);

  // --- Conversion lambda ---
  auto convert = [fromCombo, toCombo, inputSpin, outputSpin, converterType]() {
    double inputValue = inputSpin->value();
    int fromIndex = fromCombo->currentIndex();
    int toIndex = toCombo->currentIndex();

    if (fromIndex == toIndex) {
      outputSpin->setValue(inputValue);
      return;
    }

    double result = 0.0;
    switch (converterType) {
      case UnitConverterType::LENGTH:
        result = UnitConverter::convertLength(inputValue, getLengthEnum(fromIndex), getLengthEnum(toIndex));
        break;
      case UnitConverterType::TIME:
        result = UnitConverter::convertTime(inputValue, getTimeEnum(fromIndex), getTimeEnum(toIndex));
        break;
      case UnitConverterType::TEMPERATURE:
        result = UnitConverter::convertTemperature(inputValue, getTemperatureEnum(fromIndex), getTemperatureEnum(toIndex));
        break;
      case UnitConverterType::WEIGHT:
        result = UnitConverter::convertWeight(inputValue, getWeightEnum(fromIndex), getWeightEnum(toIndex));
        break;
      case UnitConverterType::CURRENCY:
        result = UnitConverter::convertCurrency(inputValue, getCurrencyEnum(fromIndex), getCurrencyEnum(toIndex));
        break;
    }

    outputSpin->setValue(result);
  };

  // --- Connect signals ---
  QObject::connect(convertBtn, &QPushButton::clicked, convert);
  QObject::connect(inputSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), convert);
  QObject::connect(fromCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), convert);
  QObject::connect(toCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), convert);

  QObject::connect(swapBtn, &QPushButton::clicked, [convert, inputSpin, outputSpin, fromCombo, toCombo]() {
    int fromIndex = fromCombo->currentIndex();
    int toIndex = toCombo->currentIndex();

    fromCombo->setCurrentIndex(toIndex);
    toCombo->setCurrentIndex(fromIndex);

    double currentOutput = outputSpin->value();
    inputSpin->setValue(currentOutput);

    convert();
  });

  convert(); // initial conversion

  return mainWidget;
}

// Utility methods
QStringList ToolWidgetManager::getAvailableTools() const {
  return s_toolFactories.keys();
}

QStringList ToolWidgetManager::getToolsByCategory(const QString& category) const {
  QStringList tools;
  for (auto it = s_toolInfo.begin(); it != s_toolInfo.end(); ++it) {
    if (it.value().category == category) {
      tools << it.key();
    }
  }
  return tools;
}

QStringList ToolWidgetManager::getCategories() const {
  QSet<QString> categories;
  for (const auto& info : s_toolInfo) {
    categories.insert(info.category);
  }
  return categories.values();
}

ToolWidgetManager::ToolInfo ToolWidgetManager::getToolInfo(const QString& toolName) const {
  return s_toolInfo.value(toolName);
}

void ToolWidgetManager::clearCache() {
  m_widgetCache.clear();
}

void ToolWidgetManager::clearCache(const QString& toolName) {
  m_widgetCache.remove(toolName);
}

int ToolWidgetManager::getCacheSize() const {
  return m_widgetCache.size();
}

// Unit name providers
QStringList ToolWidgetManager::getLengthUnitNames() {
  return {
      "Millimeters (mm)",
      "Centimeters (cm)",
      "Decimeters (dm)",
      "Meters (m)",
      "Kilometers (km)",
      "Inches (in)",
      "Feet (ft)",
      "Yard (yd)",
      "Mile (mi)"
  };
}

QStringList ToolWidgetManager::getTimeUnitNames() {
  return {
      "Milliseconds",
      "Seconds",
      "Minutes",
      "Hours"
  };
}

QStringList ToolWidgetManager::getTemperatureUnitNames() {
  return {
      "Celsius (°C)",
      "Kelvin (K)",
      "Fahrenheit (°F)"
  };
}

QStringList ToolWidgetManager::getWeightUnitNames() {
  return {
      "Milligrams (mg)",
      "Grams",
      "Kilograms",
      "Pounds (lbs)",
      "Tons",
      "Ounce (oz)"
  };
}

QStringList ToolWidgetManager::getCurrencyUnitNames() {
  return {
      "US Dollar (USD)",
      "Euro (EUR)",
      "British Pound (GBP)",
      "Turkish Lira (TRY)",
      "Japanese Yen (JPY)",
      "Swiss Franc (CHF)",
      "Canadian Dollar (CAD)",
      "Australian Dollar (AUD)",
      "Chinese Yuan (CNY)",
      "Russian Ruble (RUB)",
      "Indian Rupee (INR)",
      "Saudi Riyal (SAR)"
  };
}
