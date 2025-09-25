#include "toastmanager.h"
#include <QApplication>
#include <QEasingCurve>
#include "../etc/enums.h"
#include "../etc/apptranslations.h"

ToastWidget::ToastWidget(const QString& message, ToastType type, QWidget* parent)
    : QLabel(parent), fadeAnimation(nullptr), autoCloseTimer(new QTimer(this))
{
  setText(message);

  auto [bgColor, textColor] = getToastStyles(type);
  setStyleSheet(QString(
                    "background-color: %1; color: %2; "
                    "border-radius: 8px; padding: 12px 16px; "
                    "font-weight: 500; "
                    "border: 1px solid %3;")
                    .arg(bgColor, textColor, textColor + "40"));

  setAttribute(Qt::WA_DeleteOnClose);
  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
  adjustSize();

  // 4 saniye sonra otomatik kapanma
  autoCloseTimer->setSingleShot(true);
  connect(autoCloseTimer, &QTimer::timeout, this, &ToastWidget::startFadeOut);
  autoCloseTimer->start(4000);
}

std::pair<QString, QString> ToastWidget::getToastStyles(ToastType type) {
  switch (type) {
    case ToastType::INFO:
      return {"#E1F5FE", "#01579B"};
    case ToastType::SUCCESS:
      return {"#E8F5E8", "#2E7D32"};
    case ToastType::WARNING:
      return {"#FFF3E0", "#E65100"};
    case ToastType::ERROR:
      return {"#FFEBEE", "#C62828"};
    default:
      return {"#F5F5F5", "#424242"};
  }
}

void ToastWidget::startFadeOut() {
  emit aboutToClose();

  fadeAnimation = new QPropertyAnimation(this, "windowOpacity", this);
  fadeAnimation->setDuration(300);
  fadeAnimation->setStartValue(1.0);
  fadeAnimation->setEndValue(0.0);
  fadeAnimation->setEasingCurve(QEasingCurve::OutQuad);

  connect(fadeAnimation, &QPropertyAnimation::finished, this, &ToastWidget::onFadeOutFinished);
  fadeAnimation->start();
}

void ToastWidget::onFadeOutFinished() {
  close();
}

ToastManager::ToastManager(QWidget* parent) : QObject(parent), parentWidget(parent) {}

void ToastManager::showToast(const QString& message, ToastType type) {
  // Maksimum toast sayısını kontrol et
  if (activeToasts.size() >= MAX_TOASTS) {
    // En eski toast'ı kaldır
    if (!activeToasts.isEmpty()) {
      activeToasts.first()->startFadeOut();
    }
  }

  ToastWidget* toast = new ToastWidget(message, type, parentWidget);
  addToast(toast);
}

void ToastManager::addToast(ToastWidget* toast) {
  connect(toast, &ToastWidget::aboutToClose, this, &ToastManager::removeToast);
  connect(toast, &QObject::destroyed, this, &ToastManager::repositionToasts);

  activeToasts.append(toast);

  // İlk pozisyonu ayarla (sağ alt köşe)
  QSize parentSize = parentWidget->size();
  int x = parentSize.width() - toast->width() - MARGIN;
  int y = parentSize.height() - toast->height() - MARGIN;

  toast->move(x, y);
  toast->show();
  toast->raise();

  // Diğer toast'ları yukarı kaydır
  animateToastPositions();
}

void ToastManager::removeToast() {
  ToastWidget* sender = qobject_cast<ToastWidget*>(this->sender());
  if (sender && activeToasts.contains(sender)) {
    activeToasts.removeOne(sender);
  }
}

void ToastManager::repositionToasts() {
  QTimer::singleShot(50, this, &ToastManager::animateToastPositions);
}

void ToastManager::animateToastPositions() {
  if (activeToasts.isEmpty()) return;

  QSize parentSize = parentWidget->size();
  QParallelAnimationGroup* animGroup = new QParallelAnimationGroup(this);

  // Alt toast'tan başlayarak pozisyonları ayarla
  int currentY = parentSize.height() - MARGIN;

  for (int i = activeToasts.size() - 1; i >= 0; --i) {
    ToastWidget* toast = activeToasts[i];
    if (!toast) continue;

    int targetY = currentY - toast->height();
    int targetX = parentSize.width() - toast->width() - MARGIN;

    // Animasyon oluştur
    QPropertyAnimation* posAnim = new QPropertyAnimation(toast, "geometry");
    posAnim->setDuration(250);
    posAnim->setStartValue(toast->geometry());
    posAnim->setEndValue(QRect(targetX, targetY, toast->width(), toast->height()));
    posAnim->setEasingCurve(QEasingCurve::OutCubic);

    animGroup->addAnimation(posAnim);

    currentY = targetY - TOAST_SPACING;
  }

  // Animasyonu başlat ve kendini temizle
  connect(animGroup, &QParallelAnimationGroup::finished, animGroup, &QObject::deleteLater);
  animGroup->start();
}
