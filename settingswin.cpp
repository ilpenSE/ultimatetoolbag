#include "settingswin.h"
#include <QJsonObject>
#include <QTimer>

#include "ui_settingswin.h"
#include "etc/instances.h"
#include "etc/enums.h"
#include "etc/apptranslations.h"
#include "skeleton/loggerstream.h"

SettingsWin::SettingsWin(QJsonObject* settings, QWidget *parent)
    : QDialog(parent), m_settings(settings), ui(new Ui::SettingsWin) {
  ui->setupUi(this);
  this->setWindowTitle(_tr("Settings") + " - " + _tr("Working"));

  // Connect to theme changes
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &SettingsWin::onThemeChanged);

  setupDynamicIcons();

  // langbox init
  // those pairs are like this: VISIBLE NAME, UserRole KEY
  ui->langbox->clear();
  for (Language lang : LanguageManager::orderedLanguages) {
    QString locale = LanguageManager::languageToLocaleMap.value(lang);
    ui->langbox->addItem(getLanguageStr(lang), locale);
  }

  // theme box init
  ui->themebox->clear();
  QStringList thms = _themesman.availableThemes(); // visible names
  for (const QString& thm : std::as_const(thms)) {
    ui->themebox->addItem(_tr(thm), _themesman.getFileName(thm));
  }

  QTimer::singleShot(30, this, [this]() {
    loadSettingsUi();

    this->setWindowTitle(_tr("Settings"));
  });
}

void SettingsWin::savemem() {
  (*m_settings)["Language"] = getComboBoxData(ui->langbox);
  (*m_settings)["Theme"] = getComboBoxData(ui->themebox);
  (*m_settings)["SlideAnimation"] = getSpinBoxData(ui->slidebox);

  sinfo << "Settings object has been updated in settings menu.";
}

void SettingsWin::loadSettingsUi() {
  // set boxs
  setComboBoxByData(ui->langbox, (*m_settings)["Language"].toString());
  setComboBoxByData(ui->themebox, (*m_settings)["Theme"].toString());
  setSpinBoxByData(ui->slidebox, (*m_settings)["SlideAnimation"].toInt());

  // set patch notes
  ui->patchNotesText->setMarkdown(_forg.getPatchNotes());
}

void SettingsWin::setComboBoxByData(QComboBox *comboBox, const QVariant &data) {
  int index = comboBox->findData(data);
  if (index != -1) {
    comboBox->setCurrentIndex(index);
  } else {
    // select first item by default
    comboBox->setCurrentIndex(0);
  }
}

void SettingsWin::setSpinBoxByData(QSpinBox *spbox, const QVariant &data) {
  bool ok;
  int value = data.toInt(&ok);

  if (ok && value >= spbox->minimum() && value <= spbox->maximum()) {
    spbox->setValue(value);
    return;
  }

  spbox->setValue(spbox->minimum());
}

QJsonValue SettingsWin::getSpinBoxData(QSpinBox* spbox) {
  return QJsonValue(spbox->value());
}

QJsonValue SettingsWin::getComboBoxData(QComboBox *comboBox) {
  return QJsonValue::fromVariant(comboBox->currentData());
}

SettingsWin::~SettingsWin() { delete ui; }

void SettingsWin::lockOrUnlockUI(bool isenabled) {
  QString wrkn = " - " + _tr("Working");
  this->setEnabled(isenabled);
  ui->okbtn->setEnabled(isenabled);
  ui->cancelbtn->setEnabled(isenabled);
  this->setWindowTitle(_tr("Settings") + (isenabled ? "" : wrkn));
}

void SettingsWin::on_refreshthbtn_clicked() {
  QString theme = (*m_settings)["Theme"].toString();
  thinfo << "Refreshing the theme: " + theme;

  lockOrUnlockUI(false);
  if (!_themesman.applyTheme(theme)) {
    therr << "Cannot apply theme: " + theme;
  }
  lockOrUnlockUI(true);

  thinfo << "Theme " + theme + " refreshed!";
}

void SettingsWin::setupDynamicIcons() {
  QString iconsPath = ":/assets/icons/";
  int s = 20;
  // refresh btn
  if (ui->refreshthbtn) {
    _themesman.setupDynamicButton(
        ui->refreshthbtn, iconsPath + "refresh.svg", QSize(s, s));
  }
  thinfo << "(SettingsWin) Dynamic icons setup completed";
}

void SettingsWin::onThemeChanged() {
  refreshIcons();
}

void SettingsWin::refreshIcons() {
  // Force refresh all icons if needed
  _themesman.refreshAllIcons();
}

