#ifndef SETTINGSWIN_H
#define SETTINGSWIN_H

#include <QDialog>
#include <QJsonObject>
#include <QComboBox>
#include <QSpinBox>

namespace Ui {
class SettingsWin;
}

class SettingsWin : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsWin(QJsonObject* settings, QWidget *parent = nullptr);
  ~SettingsWin();

 private slots:
  void savemem();

  void on_refreshthbtn_clicked();
  void onThemeChanged();

 private:
  void setupDynamicIcons();
  void refreshIcons();
  void loadSettingsUi();

  // combo box
  void setComboBoxByData(QComboBox *comboBox, const QVariant &data);
  QJsonValue getComboBoxData(QComboBox *comboBox);

  // spin box
  void setSpinBoxByData(QSpinBox *spBox, const QVariant &data);
  QJsonValue getSpinBoxData(QSpinBox* spbox);

  void lockOrUnlockUI(bool isEnabled);
  QJsonObject* m_settings;
  Ui::SettingsWin *ui;
};

#endif  // SETTINGSWIN_H
