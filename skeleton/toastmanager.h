#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QWidget>
#include <QLabel>
#include <QList>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QVBoxLayout>
#include "../etc/enums.h"

class ToastWidget : public QLabel {
  Q_OBJECT

 public:
  explicit ToastWidget(const QString& message, ToastType type, QWidget* parent = nullptr);

  void startFadeOut();

 signals:
  void aboutToClose();

 private slots:
  void onFadeOutFinished();

 private:
  static std::pair<QString, QString> getToastStyles(ToastType type);
  QPropertyAnimation* fadeAnimation;
  QTimer* autoCloseTimer;
};

class ToastManager : public QObject {
  Q_OBJECT

 public:
  explicit ToastManager(QWidget* parent);
  void showToast(const QString& message, ToastType type = ToastType::INFO);

 private slots:
  void removeToast();
  void repositionToasts();

 private:
  QWidget* parentWidget;
  QList<ToastWidget*> activeToasts;
  static constexpr int TOAST_SPACING = 10;
  static constexpr int MARGIN = 15;
  static constexpr int MAX_TOASTS = 5;

  void addToast(ToastWidget* toast);
  void animateToastPositions();
};

#endif // TOASTMANAGER_H
