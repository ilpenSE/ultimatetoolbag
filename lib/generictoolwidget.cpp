#include "generictoolwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QHash>
#include <functional>

#include "../etc/apptranslations.h"

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
  toolMap[toolName](this);
}

void GenericToolWidget::wplaceholder() {
  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* label = new QLabel(_tr("Please wait"), this);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
}

void GenericToolWidget::wpdfword()       { qDebug() << "PDF Word açıldı"; }
void GenericToolWidget::wziprar()        { qDebug() << "ZIP/RAR açıldı"; }
void GenericToolWidget::wjsonprettier()  { qDebug() << "JSON Prettier açıldı"; }
void GenericToolWidget::wcurl()          { qDebug() << "CURL açıldı"; }
void GenericToolWidget::whashgen()       { qDebug() << "Hash Generator açıldı"; }
void GenericToolWidget::waesrsa()        { qDebug() << "AES/RSA açıldı"; }
void GenericToolWidget::wbaseencoder()   { qDebug() << "Base Encoder açıldı"; }
void GenericToolWidget::wcolorpicker()   { qDebug() << "Color Picker açıldı"; }
void GenericToolWidget::wregextester()   { qDebug() << "Regex Tester açıldı"; }
void GenericToolWidget::wapitester()     { qDebug() << "API Tester açıldı"; }
void GenericToolWidget::wlength()        { qDebug() << "Length Converter açıldı"; }
void GenericToolWidget::wtime()          { qDebug() << "Time Converter açıldı"; }
void GenericToolWidget::wweight()        { qDebug() << "Weight Converter açıldı"; }
void GenericToolWidget::wtemperature()   { qDebug() << "Temperature Converter açıldı"; }
void GenericToolWidget::wcurrency()      { qDebug() << "Currency Converter açıldı"; }
void GenericToolWidget::wimagefixer()    { qDebug() << "Image Fixer açıldı"; }
void GenericToolWidget::wjsonfixer()     { qDebug() << "JSON Fixer açıldı"; }

// DESTRUCTOR
GenericToolWidget::~GenericToolWidget() {}
