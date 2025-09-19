#include "iconutils.h"

#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QPainter>

#include "logger.h"
#include "thememanager.h"
#include "loggerstream.h"
#include "../etc/instances.h"

IconUtils::IconUtils(QObject* parent) : QObject(parent) {}

void IconUtils::setupToolbarIcons(QWidget* toolbar,
                                  const QMap<QString, QString>& actionIconMap) {
  if (!toolbar) return;

  QString iconsPath = ":/assets/icons";  // Use Qt resource system
  int successCount = 0;
  int totalCount = actionIconMap.size();

  for (auto it = actionIconMap.constBegin(); it != actionIconMap.constEnd();
       ++it) {
    QString actionName = it.key();
    QString iconFileName = it.value();

    QAction* action = toolbar->findChild<QAction*>(actionName);
    if (action) {
      QString fullIconPath = iconsPath + "/" + iconFileName;
      if (setupIcon(action, fullIconPath)) {
        successCount++;
      }
    }
  }

  thinfo << QString("Toolbar icons setup: %1/%2 successful").arg(successCount, totalCount);
}

void IconUtils::setupButtonIcons(QWidget* parent,
                                 const QMap<QString, QString>& buttonIconMap) {
  if (!parent) return;

  QString iconsPath = ":/assets/icons";  // Use Qt resource system
  int successCount = 0;
  int totalCount = buttonIconMap.size();

  for (auto it = buttonIconMap.constBegin(); it != buttonIconMap.constEnd();
       ++it) {
    QString buttonName = it.key();
    QString iconFileName = it.value();

    // Try QPushButton first
    QPushButton* pushButton = parent->findChild<QPushButton*>(buttonName);
    if (pushButton) {
      QString fullIconPath = iconsPath + "/" + iconFileName;
      if (setupIcon(pushButton, fullIconPath)) {
        successCount++;
        continue;
      }
    }

    // Try QToolButton
    QToolButton* toolButton = parent->findChild<QToolButton*>(buttonName);
    if (toolButton) {
      QString fullIconPath = iconsPath + "/" + iconFileName;
      if (setupIcon(toolButton, fullIconPath)) {
        successCount++;
      }
    }
  }

  thinfo << QString("Button icons setup: %1/%2 successful").arg(successCount, totalCount);
}

bool IconUtils::setupIcon(QAction* action, const QString& iconPath,
                          const QSize& size) {
  if (!action) return false;

  if (validateIconPath(iconPath)) {
    _themesman.setupDynamicAction(action, iconPath, size);
    return true;
  } else {
    // Create fallback icon
    QString actionText = action->text().remove('&');  // Remove mnemonics
    if (actionText.isEmpty()) actionText = "?";
    QIcon fallbackIcon = createFallbackIcon(actionText.left(2), size);
    action->setIcon(fallbackIcon);
    thwrn << "Fallback icon created for action: " + action->objectName();
    return false;
  }
}

bool IconUtils::setupIcon(QPushButton* button, const QString& iconPath,
                          const QSize& size) {
  if (!button) return false;

  if (validateIconPath(iconPath)) {
    _themesman.setupDynamicButton(button, iconPath, size);
    return true;
  } else {
    // Create fallback icon
    QString buttonText = button->text().remove('&');
    if (buttonText.isEmpty()) buttonText = "?";
    QIcon fallbackIcon = createFallbackIcon(buttonText.left(2), size);
    button->setIcon(fallbackIcon);
    button->setIconSize(size);
    thwrn << "Fallback icon created for button: " + button->objectName();
    return false;
  }
}

bool IconUtils::setupIcon(QToolButton* button, const QString& iconPath,
                          const QSize& size) {
  if (!button) return false;

  if (validateIconPath(iconPath)) {
    _themesman.setupDynamicButton(button, iconPath, size);
    return true;
  } else {
    // Create fallback icon
    QString buttonText = button->text().remove('&');
    if (buttonText.isEmpty()) buttonText = "?";
    QIcon fallbackIcon = createFallbackIcon(buttonText.left(2), size);
    button->setIcon(fallbackIcon);
    button->setIconSize(size);
    thwrn << "Fallback icon created for tool button: " + button->objectName();
    return false;
  }
}

bool IconUtils::validateIconPath(const QString& iconPath) const {
  // For Qt resource system paths
  if (iconPath.startsWith(":/")) {
    QFileInfo resourceInfo(iconPath);
    return QFile::exists(iconPath) &&
           (resourceInfo.suffix().toLower() == "svg" ||
            resourceInfo.suffix().toLower() == "png" ||
            resourceInfo.suffix().toLower() == "jpg" ||
            resourceInfo.suffix().toLower() == "jpeg");
  }

  // For regular file system paths
  QFileInfo fileInfo(iconPath);
  return fileInfo.exists() && fileInfo.isFile() &&
         (fileInfo.suffix().toLower() == "svg" ||
          fileInfo.suffix().toLower() == "png" ||
          fileInfo.suffix().toLower() == "jpg" ||
          fileInfo.suffix().toLower() == "jpeg");
}

QStringList IconUtils::findAvailableIcons(const QString& iconDirectory) const {
  QStringList iconFiles;
  QDir dir(iconDirectory);

  if (!dir.exists()) {
    fswrn << "Icon directory does not exist: " + iconDirectory;
    return iconFiles;
  }

  QStringList filters;
  filters << "*.svg" << "*.png" << "*.jpg" << "*.jpeg";

  QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
  for (const QFileInfo& file : std::as_const(files)) {
    iconFiles << file.absoluteFilePath();
  }

  return iconFiles;
}

QIcon IconUtils::createFallbackIcon(const QString& text, const QSize& size,
                                    const QColor& bgColor) {
  QPixmap pixmap(size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background
  painter.setBrush(bgColor);
  painter.setPen(Qt::NoPen);
  painter.drawRoundedRect(pixmap.rect(), 4, 4);

  // Text
  painter.setPen(Qt::white);
  QFont font = painter.font();
  font.setPixelSize(qMax(8, size.height() / 3));
  font.setBold(true);
  painter.setFont(font);
  painter.drawText(pixmap.rect(), Qt::AlignCenter, text.toUpper());

  return QIcon(pixmap);
}
