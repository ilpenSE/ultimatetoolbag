#include "generictoolwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QHash>
#include <functional>

#include "../etc/apptranslations.h"
#include "unitconverter.h"
#include "../skeleton/loggerstream.h"

QHash<QString, std::function<void(GenericToolWidget*)>> GenericToolWidget::toolMap = {
    {"pdfword", [](GenericToolWidget* self){ self->wpdfword(); }},
    {"ziprar", [](GenericToolWidget* self){ self->wziprar(); }},
    {"jsonprettier", [](GenericToolWidget* self){ self->wjsonprettier(); }},
    {"curl", [](GenericToolWidget* self){ self->wcurl(); }},
    {"hashgen", [](GenericToolWidget* self){ self->whashgen(); }},
    {"aesrsa", [](GenericToolWidget* self){ self->waesrsa(); }},
    {"baseencoder", [](GenericToolWidget* self){ self->wbaseencoder(); }},
    {"colorpicker", [](GenericToolWidget* self){ self->wcolorpicker(); }},
    {"regextester", [](GenericToolWidget* self){ self->wregextester(); }},
    {"apitester", [](GenericToolWidget* self){ self->wapitester(); }},
    {"length", [](GenericToolWidget* self){ self->wlength(); }},
    {"time", [](GenericToolWidget* self){ self->wtime(); }},
    {"weight", [](GenericToolWidget* self){ self->wweight(); }},
    {"temperature", [](GenericToolWidget* self){ self->wtemperature(); }},
    {"currency", [](GenericToolWidget* self){ self->wcurrency(); }},
    {"imagefixer", [](GenericToolWidget* self){ self->wimagefixer(); }},
    {"jsonfixer", [](GenericToolWidget* self){ self->wjsonfixer(); }},
};

GenericToolWidget::GenericToolWidget(QWidget* parent)
    : QWidget(parent)
{
  // placeholder
  wplaceholder();
}

void GenericToolWidget::showWidget(const QString& toolName) {
  if (!toolMap.contains(toolName)) return;
  toolMap[toolName](this);
}

void GenericToolWidget::wplaceholder() {
  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* label = new QLabel(_tr("Please wait"), this);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
}

void GenericToolWidget::wpdfword()       { linfo << "PDF Word açıldı"; }
void GenericToolWidget::wziprar()        { linfo << "ZIP/RAR açıldı"; }
void GenericToolWidget::wjsonprettier()  { linfo << "JSON Prettier açıldı"; }
void GenericToolWidget::wcurl()          { linfo << "CURL açıldı"; }
void GenericToolWidget::whashgen()       { linfo << "Hash Generator açıldı"; }
void GenericToolWidget::waesrsa()        { linfo << "AES/RSA açıldı"; }
void GenericToolWidget::wbaseencoder()   { linfo << "Base Encoder açıldı"; }
void GenericToolWidget::wcolorpicker()   { linfo << "Color Picker açıldı"; }
void GenericToolWidget::wregextester()   { linfo << "Regex Tester açıldı"; }
void GenericToolWidget::wapitester()     { linfo << "API Tester açıldı"; }

void GenericToolWidget::wlength() {
  createUnitConverterWidget(
      "Uzunluk Dönüştürücü",
      getLengthUnitNames(),
      "length"
      );
  linfo << "Length Converter açıldı";
}

void GenericToolWidget::wtime() {
  createUnitConverterWidget(
      "Zaman Dönüştürücü",
      getTimeUnitNames(),
      "time"
      );
  linfo << "Time Converter açıldı";
}

void GenericToolWidget::wtemperature() {
  createUnitConverterWidget(
      "Sıcaklık Dönüştürücü",
      getTemperatureUnitNames(),
      "temperature"
      );
  linfo << "Temperature Converter açıldı";
}

void GenericToolWidget::wweight()        { linfo << "Weight Converter açıldı"; }
void GenericToolWidget::wcurrency()      { linfo << "Currency Converter açıldı"; }

void GenericToolWidget::wimagefixer()    { linfo << "Image Fixer açıldı"; }
void GenericToolWidget::wjsonfixer()     { linfo << "JSON Fixer açıldı"; }

// ui helpers
QWidget* GenericToolWidget::createUnitConverterWidget(const QString& title,
                                                      const QStringList& units,
                                                      const QString& converterType) {
  // Clear existing widget
  QLayout* oldLayout = this->layout();
  if (oldLayout) {
    QLayoutItem* item;
    while ((item = oldLayout->takeAt(0)) != nullptr) {
      delete item->widget();
      delete item;
    }
    delete oldLayout;
  }

  // Main widget
  QWidget* mainWidget = new QWidget;
  QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
  this->setLayout(new QVBoxLayout);
  this->layout()->addWidget(mainWidget);

  // Group box
  QGroupBox* groupBox = new QGroupBox(title);

  QGridLayout* gridLayout = new QGridLayout(groupBox);
  gridLayout->setSpacing(15);

  // Input section
  QLabel* fromLabel = new QLabel("Dönüştür:");
  fromLabel->setStyleSheet("font-weight: bold; color: #555;");

  QDoubleSpinBox* inputSpin = new QDoubleSpinBox;
  inputSpin->setRange(-999999999, 999999999);
  inputSpin->setDecimals(6);
  inputSpin->setValue(1.0);

  QComboBox* fromCombo = new QComboBox;
  fromCombo->addItems(units);

  // Output section
  QLabel* toLabel = new QLabel("Sonuç:");
  toLabel->setStyleSheet("font-weight: bold; color: #555;");

  QDoubleSpinBox* outputSpin = new QDoubleSpinBox;
  outputSpin->setRange(-999999999, 999999999);
  outputSpin->setDecimals(6);
  outputSpin->setReadOnly(true);

  QComboBox* toCombo = new QComboBox;
  toCombo->addItems(units);
  if (units.size() > 1) toCombo->setCurrentIndex(1);
  toCombo->setStyleSheet(fromCombo->styleSheet());

  // Swap button
  QPushButton* swapBtn = new QPushButton("⇄");
  swapBtn->setToolTip("Birimleri değiştir");
  swapBtn->setMaximumWidth(60);

  // Convert button
  QPushButton* convertBtn = new QPushButton("Dönüştür");

  // Result display
  QLabel* resultLabel = new QLabel;
  resultLabel->setAlignment(Qt::AlignCenter);
  resultLabel->setText("Sonuç burada görünecek");

  // Layout arrangement
  gridLayout->addWidget(fromLabel, 0, 0);
  gridLayout->addWidget(inputSpin, 1, 0);
  gridLayout->addWidget(fromCombo, 1, 1);
  gridLayout->addWidget(swapBtn, 1, 2, Qt::AlignCenter);

  gridLayout->addWidget(toLabel, 2, 0);
  gridLayout->addWidget(outputSpin, 3, 0);
  gridLayout->addWidget(toCombo, 3, 1);
  gridLayout->addWidget(convertBtn, 3, 2);

  gridLayout->addWidget(resultLabel, 4, 0, 1, 3);

  // Add some spacing
  gridLayout->setRowMinimumHeight(0, 10);
  gridLayout->setRowMinimumHeight(2, 20);

  mainLayout->addWidget(groupBox);
  mainLayout->addStretch();

  auto convert = [fromCombo, toCombo, inputSpin, outputSpin, resultLabel, converterType]() {
    if (!fromCombo || !toCombo || !inputSpin || !outputSpin || !resultLabel) return;

    double inputValue = inputSpin->value();
    int fromIndex = fromCombo->currentIndex();
    int toIndex = toCombo->currentIndex();

    if (fromIndex == toIndex) {
      outputSpin->setValue(inputValue);
      resultLabel->setText(QString("%1 = %2").arg(inputValue, inputValue));
      return;
    }

    double result = 0.0;

    // Use your backend functions based on converter type
    if (converterType == "length") {
      result = UnitConverter::convertLength(inputValue, getLengthEnum(fromIndex), getLengthEnum(toIndex));
    }
    else if (converterType == "time") {
      result = UnitConverter::convertTime(inputValue, getTimeEnum(fromIndex), getTimeEnum(toIndex));
    }
    else if (converterType == "temperature") {
      result = UnitConverter::convertTemperature(inputValue, getTemperatureEnum(fromIndex), getTemperatureEnum(toIndex));
    }

    outputSpin->setValue(result);

    QString fromUnit = fromCombo->currentText();
    QString toUnit = toCombo->currentText();

    // Format the result nicely
    QString formattedInput = QString::number(inputValue, 'f', inputValue == (int)inputValue ? 0 : 3);
    QString formattedResult = QString::number(result, 'f', result == (int)result ? 0 : 6);

    resultLabel->setText(QString("%1 %2 = %3 %4")
                             .arg(formattedInput, fromUnit.split('(').first().trimmed(),
                                  formattedResult,toUnit.split('(').first().trimmed()));
  };

  // Connect signals
  connect(convertBtn, &QPushButton::clicked, convert);
  connect(inputSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), convert);
  connect(fromCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), convert);
  connect(toCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), convert);

  // Swap functionality
  connect(swapBtn, &QPushButton::clicked, this, [convert, outputSpin, inputSpin, fromCombo, toCombo]() {
    if (!outputSpin || !inputSpin || !fromCombo || !toCombo) return;

    int fromIndex = fromCombo->currentIndex();
    int toIndex = toCombo->currentIndex();

    fromCombo->setCurrentIndex(toIndex);
    toCombo->setCurrentIndex(fromIndex);

    double currentOutput = outputSpin->value();
    inputSpin->setValue(currentOutput);

    convert();
  });

  // Initial conversion
  convert();

  return mainWidget;
}

// DESTRUCTOR
GenericToolWidget::~GenericToolWidget() {}
