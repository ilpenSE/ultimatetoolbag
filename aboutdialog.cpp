#include "aboutdialog.h"

#include "ui_aboutdialog.h"
#include "etc/apptranslations.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
  ui->setupUi(this);

  this->setWindowTitle(_tr("About"));

  connect(ui->buttonOK, &QPushButton::clicked, this, &QDialog::accept);

  ui->textAbt->setHtml(createLocalizedAboutText());
}

QString AboutDialog::createLocalizedAboutText() {
  QString htmlTemplate = R"(
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/_tr/REC-html40/s_trict.dtd">
<html><head><meta name="qrichtext" content="1" /><meta charset="utf-8" />
<style type="text/css">
p, li { white-space: pre-wrap; }
hr { height: 1px; border-width: 0; }
li.unchecked::marker { content: "\2610"; }
li.checked::marker { content: "\2612"; }
</style></head>
<body style=" font-family:'Segoe UI'; font-size:9pt; font-weight:400; font-style:normal;">
<img src=":/assets/icons/appicon.svg" width="128" height="128" align="right" hspace="10" />

<h1 style=" margin-top:18px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:11pt; font-weight:700;">Ultimate Toolbag</span></h1>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%1: </span>
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt; font-weight:700;">1.0.0</span></p>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%2: </span>
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt; font-weight:700;">ilpeN</span></p>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%3: </span>
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt; font-weight:700;">GNU General Public License v3.0 (GPLv3)</span></p>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%4</span></p>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%5</span></p>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt;">%6</span></p>

<h2 style=" margin-top:16px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<span style=" font-family:'Segoe UI','sans-serif'; font-size:10pt; font-weight:700;">%7</span></h2>

<p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">
<a href="https://github.com/ilpenSE"><span style=" font-size:10pt; text-decoration: underline; color:#27bf73;">GitHub</span></a> |
<a href="https://instagram.com/ilpen.25"><span style=" font-size:10pt; text-decoration: underline; color:#27bf73;">Instagram</span></a> |
<a href="https://x.com/ilpenwastaken"><span style=" font-size:10pt; text-decoration: underline; color:#27bf73;">X</span></a> |
<a href="https://youtube.com/@ilpenwastaken"><span style=" font-size:10pt; text-decoration: underline; color:#27bf73;">YouTube</span></a></p>
</body></html>
)";

  return htmlTemplate
      .arg(tr("Version"))           // %1
      .arg(tr("Author"))            // %2
      .arg(tr("License"))           // %3
      .arg(tr("Built with Qt (Free Edition)"))  // %4
      .arg(tr("This application uses the Qt framework (LGPLv3). Qt is a free and open-source project by The Qt Company and contributors."))  // %5
      .arg(tr("Made in Türkiye"))    // %6
      .arg(tr("Links"));             // %7
}

AboutDialog::~AboutDialog() { delete ui; }
