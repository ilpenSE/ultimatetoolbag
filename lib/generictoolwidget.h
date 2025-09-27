#ifndef GENERICTOOLWIDGET_H
#define GENERICTOOLWIDGET_H

#include <QObject>
#include <QHash>
#include <functional>
#include <QtWidgets>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QColorDialog>
#include <QSlider>
#include <QCheckBox>
#include <QTabWidget>
#include <QGroupBox>
#include <QSplitter>
#include <QTableWidget>
#include <QTreeWidget>
#include <QPlainTextEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QCalendarWidget>
#include <QDateTimeEdit>

class GenericToolWidget : public QWidget {
 public:
  explicit GenericToolWidget(QWidget* parent = nullptr);
  ~GenericToolWidget();
  void showWidget(const QString& toolName);

 private:
  static QHash<QString, std::function<void(GenericToolWidget*)>> toolMap;
  void wplaceholder();

  void wpdfword();
  void wziprar();
  void wjsonprettier();
  void wcurl();
  void whashgen();
  void waesrsa();
  void wbaseencoder();
  void wcolorpicker();
  void wregextester();
  void wapitester();
  void wlength();
  void wtime();
  void wweight();
  void wtemperature();
  void wcurrency();
  void wimagefixer();
  void wjsonfixer();

  QWidget* createUnitConverterWidget(const QString& title, const QStringList& units,
                                                        const QString& converterType);

  QStringList getLengthUnitNames() {
    return {
        "Millimeters (mm)",
        "Centimeters (cm)",
        "Desimeters (dm)",
        "Meters (m)",
        "Kilometers (km)",
        "Inches (in)",
        "Feet (ft)",
        "Yard (yd)",
        "Mile (mi)"
    };
  }

  QStringList getTimeUnitNames() {
    return {
        "Milliseconds",
        "Seconds",
        "Minutes",
        "Hours"
    };
  }

  QStringList getTemperatureUnitNames() {
    return {
        "Celsius (°C)",
        "Kelvin (K)",
        "Fahrenheit (°F)"
    };
  }
};

#endif  // GENERICTOOLWIDGET_H
