#ifndef GENERICTOOLWIDGET_H
#define GENERICTOOLWIDGET_H

#include <QObject>
#include <QWidget>
#include <QHash>
#include <functional>

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
};

#endif  // GENERICTOOLWIDGET_H
