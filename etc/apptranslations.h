#ifndef APPTRANSLATIONS_H
#define APPTRANSLATIONS_H

#include <QString>
#include <QCoreApplication>
#include "../mainwindow.h"

#define _tr(x) MainWindow::sftr(x)

template<typename... Args>
inline QString _trf(const char* sourceText, Args&&... args) {
  return MainWindow::sftr(sourceText).arg(args...);
}

#endif  // APPTRANSLATIONS_H
