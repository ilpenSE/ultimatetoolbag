#include "aboutdialog.h"

#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  this->setWindowTitle(trans("About"));

  connect(ui->buttonOK, &QPushButton::clicked, this, &QDialog::accept);
}

QString AboutDialog::trans(const QString& k) {
  return QApplication::translate("MainWindow", qPrintable(k));
}

AboutDialog::~AboutDialog() { delete ui; }
