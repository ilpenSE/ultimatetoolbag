#include "thememanager.h"

#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QJsonObject>
#include <QPainter>
#include <QPalette>
#include <QPointer>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStyle>
#include <QWidget>
#include <QtSvg/QSvgRenderer>

#include "loggerstream.h"
#include "logger.h"
#include "../etc/instances.h"

QMap<QString, QString> themeMap;
QMap<QString, QString> reversedThemeMap;

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
  globalQss = readQssFile(":/assets/global.qss");
  loadThemesFromJson();
  reversedThemeMap = reverseThemeMap();
}

QString ThemeManager::themesDirPath() const {
  QString path = _forg.vAssetPath();
  QDir dir(path + "/themes");
  if (!dir.exists()) {
    dir.mkpath(".");
    fswrn << "Themes folder not found, creating one...";
  }
  return dir.absolutePath();
}

QString ThemeManager::themesJsonPath() const {
  QString path = _forg.appFolderPath();
  return path + "/themes.json";
}

QString ThemeManager::readQssFile(const QString& filePath) const {
  QFile file(filePath);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
    fserr << "Theme file cannot be opened: " + filePath;
    return QString();
  }
  return QString::fromUtf8(file.readAll());
}

void ThemeManager::resolveVars(QString* qssContent) {
  if (!qssContent) return;

  QString& qss = *qssContent;

  // :root {...} bloğunu bul ve değişkenleri çıkar
  QRegularExpression rootRegex(R"(:root\s*\{([^}]+)\})");
  QRegularExpressionMatch rootMatch = rootRegex.match(qss);

  QMap<QString, QString> varMap;

  if (rootMatch.hasMatch()) {
    QString rootContent = rootMatch.captured(1);

    // CSS değişkenlerini parse et: --variable-name: value;
    QRegularExpression declRegex(R"(--([a-zA-Z0-9_-]+)\s*:\s*([^;]+);)");
    QRegularExpressionMatchIterator it = declRegex.globalMatch(rootContent);

    while (it.hasNext()) {
      QRegularExpressionMatch match = it.next();
      QString key = match.captured(1).trimmed();
      QString value = match.captured(2).trimmed();
      varMap[key] = value;
      thinfo << QString("CSS Variable parsed: --%1: %2").arg(key, value);
    }
  }

  // var(--key) kullanımlarını değiştir
  QRegularExpression useRegex(R"(var\(--([a-zA-Z0-9_-]+)\))");
  QRegularExpressionMatchIterator useIt = useRegex.globalMatch(qss);

  // Geriye doğru değiştir ki offset'ler bozulmasın
  QList<QRegularExpressionMatch> matches;
  while (useIt.hasNext()) {
    matches.append(useIt.next());
  }

  for (int i = matches.size() - 1; i >= 0; --i) {
    QRegularExpressionMatch match = matches[i];
    QString key = match.captured(1);
    if (varMap.contains(key)) {
      qss.replace(match.capturedStart(), match.capturedLength(), varMap[key]);
    }
  }

  // :root bloğunu tamamen kaldır
  qss.remove(rootRegex);

  thinfo << "CSS variables resolved and :root block removed";
}

void ThemeManager::loadThemesFromJson() {
  themeMap.clear();

  QJsonObject loaded = _forg.loadJson(themesJsonPath());
  if (loaded.isEmpty()) {
    fserr << "Cannot load themes.json!";
    return;
  }

  for (auto it = loaded.begin(); it != loaded.end(); ++it) {
    QString qssFile = it.key();
    QString visibleName = it.value().toString();
    themeMap[visibleName] = qssFile;
    thinfo << "Theme loaded, visible name: " + visibleName +
                    " / file name: " + qssFile;
  }

  thinfo << "Loaded themes from themes.json";
}

QPixmap ThemeManager::coloredPixmap(const QString& svgPath, const QColor& color,
                                    const QSize& size) {
  if (!QFile::exists(svgPath) || size.isEmpty())
    return QPixmap();

  QSvgRenderer renderer(svgPath);
  if (!renderer.isValid())
    return QPixmap();

  const qreal dpr = qApp->devicePixelRatio();
  QPixmap pix(size * dpr);
  pix.setDevicePixelRatio(dpr);
  pix.fill(Qt::transparent);

  QPainter painter(&pix);
  painter.setRenderHint(QPainter::Antialiasing);
  renderer.render(&painter);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pix.rect(), color);

  return pix;
}

QIcon ThemeManager::createDynamicIcon(const QString& svgPath,
                                      const QSize& size) {
  QIcon icon;

  // Normal state
  icon.addPixmap(coloredPixmap(svgPath, m_iconColor, size), QIcon::Normal,
                 QIcon::Off);

  // Hover state (Active)
  icon.addPixmap(coloredPixmap(svgPath, m_hoverColor, size), QIcon::Active,
                 QIcon::Off);

  // Pressed state (Selected)
  icon.addPixmap(coloredPixmap(svgPath, m_pressedColor, size), QIcon::Selected,
                 QIcon::Off);

  // Disabled state
  icon.addPixmap(coloredPixmap(svgPath, m_disabledColor, size), QIcon::Disabled,
                 QIcon::Off);

  return icon;
}

void ThemeManager::setupDynamicAction(QAction* action, const QString& svgPath,
                                      const QSize& size) {
  if (!action) return;

  // Register the icon for updates
  IconData iconData;
  iconData.svgPath = svgPath;
  iconData.size = size;
  iconData.widget = action;
  m_registeredIcons[action] = iconData;

  // Set initial icon
  action->setIcon(createDynamicIcon(svgPath, size));

  thinfo << "Dynamic icon setup for QAction: " + svgPath;
}

void ThemeManager::setupDynamicButton(QPushButton* button,
                                      const QString& svgPath,
                                      const QSize& size) {
  if (!button) return;

  // Register the icon for updates
  IconData iconData;
  iconData.svgPath = svgPath;
  iconData.size = size;
  iconData.widget = button;
  m_registeredIcons[button] = iconData;

  // Set initial icon
  button->setIcon(createDynamicIcon(svgPath, size));
  button->setIconSize(size);

  thinfo << "Dynamic icon setup for QPushButton: " + svgPath;
}

void ThemeManager::setupDynamicButton(QToolButton* button,
                                      const QString& svgPath,
                                      const QSize& size) {
  if (!button) return;

  // Register the icon for updates
  IconData iconData;
  iconData.svgPath = svgPath;
  iconData.size = size;
  iconData.widget = button;
  m_registeredIcons[button] = iconData;

  // Set initial icon
  button->setIcon(createDynamicIcon(svgPath, size));
  button->setIconSize(size);

  thinfo << "Dynamic icon setup for QToolButton: " + svgPath;
}

void ThemeManager::setIconColors(const QColor& normal, const QColor& hover,
                                 const QColor& pressed,
                                 const QColor& disabled) {
  m_iconColor = normal;
  m_hoverColor = hover;
  m_pressedColor = pressed;
  m_disabledColor = disabled;

  // Mevcut iconları güncelle
  updateIconColors();

  thinfo << "Icon colors updated programmatically";
}

void ThemeManager::parseThemeColors(const QString& qssContent) {
  // İlk olarak yorumlardan icon renklerini çıkar
  static QRegularExpression iconColorRegex(
      R"(\/\*\s*@icon-color:\s*([^*]+)\s*\*\/)");
  static QRegularExpression hoverColorRegex(
      R"(\/\*\s*@hover-color:\s*([^*]+)\s*\*\/)");
  static QRegularExpression pressedColorRegex(
      R"(\/\*\s*@pressed-color:\s*([^*]+)\s*\*\/)");
  static QRegularExpression disabledColorRegex(
      R"(\/\*\s*@disabled-color:\s*([^*]+)\s*\*\/)");

  // Defaults
  m_iconColor = QColor(204, 204, 204);
  m_hoverColor = QColor(255, 255, 255);
  m_pressedColor = QColor(136, 136, 136);
  m_disabledColor = QColor(85, 85, 85);

  // Light tema tespiti için QSS içeriğini kontrol et
  bool isLightTheme =
      qssContent.contains("light", Qt::CaseInsensitive) ||
      (qssContent.contains("#ffffff") && !qssContent.contains("#1e1e1e"));

  if (isLightTheme) {
    m_iconColor = QColor(51, 51, 51);
    m_hoverColor = QColor(0, 120, 212);
    m_pressedColor = QColor(0, 90, 158);
    m_disabledColor = QColor(136, 136, 136);
  }

  // Yorumlardan custom renkleri parse et
  auto iconMatch = iconColorRegex.match(qssContent);
  if (iconMatch.hasMatch()) {
    QString colorStr = iconMatch.captured(1).trimmed();
    QColor color(colorStr);
    if (color.isValid()) {
      m_iconColor = color;
    }
  }

  auto hoverMatch = hoverColorRegex.match(qssContent);
  if (hoverMatch.hasMatch()) {
    QString colorStr = hoverMatch.captured(1).trimmed();
    QColor color(colorStr);
    if (color.isValid()) {
      m_hoverColor = color;
    }
  }

  auto pressedMatch = pressedColorRegex.match(qssContent);
  if (pressedMatch.hasMatch()) {
    QString colorStr = pressedMatch.captured(1).trimmed();
    QColor color(colorStr);
    if (color.isValid()) {
      m_pressedColor = color;
    }
  }

  auto disabledMatch = disabledColorRegex.match(qssContent);
  if (disabledMatch.hasMatch()) {
    QString colorStr = disabledMatch.captured(1).trimmed();
    QColor color(colorStr);
    if (color.isValid()) {
      m_disabledColor = color;
    }
  }

  thinfo << QString(
                  "Theme colors parsed - Icon: %1, Hover: %2, Pressed: %3, "
                  "Disabled: %4")
                  .arg(m_iconColor.name(), m_hoverColor.name(), m_pressedColor.name(), m_disabledColor.name());
}

bool ThemeManager::applyTheme(const QString& fileName) {
  if (!reversedThemeMap.contains(fileName)) {
    therr << "Theme not found: " + fileName;
    return false;
  }

  QString filePath = themesDirPath() + "/" + fileName + ".qss";
  if (!QFile::exists(filePath)) {
    fserr << "Theme file cannot be found: " + filePath;
    return false;
  }

  QString themeQss = readQssFile(filePath);
  if (themeQss.isEmpty()) {
    therr << "QSS file is empty.";
    return false;
  }

  // merge global and theme files
  QString mergedQss = globalQss + "\n" + themeQss;

  resolveVars(&mergedQss);
  // Parse theme colors before applying
  parseThemeColors(mergedQss);

  allQss = mergedQss;

  qApp->setStyleSheet(mergedQss);
  thinfo << "Theme applied: " + fileName;

  // Update all registered icons
  updateIconColors();
  emit themeChanged();

  return true;
}

QString ThemeManager::getAllQss() {
  return allQss;
}

void ThemeManager::updateIconColors() {
  for (auto it = m_registeredIcons.begin(); it != m_registeredIcons.end();) {
    QPointer<QObject> widget = it.value().widget;

    // Eğer widget artık yoksa kayıtları sil
    if (!widget) {
      it = m_registeredIcons.erase(it);
      continue;
    }

    // Eğer widget hâlâ geçerli ise icon güncelle
    QIcon newIcon = createDynamicIcon(it.value().svgPath, it.value().size);

    if (QAction* action = qobject_cast<QAction*>(widget)) {
      action->setIcon(newIcon);
    } else if (QPushButton* button = qobject_cast<QPushButton*>(widget)) {
      button->setIcon(newIcon);
    } else if (QToolButton* toolButton = qobject_cast<QToolButton*>(widget)) {
      toolButton->setIcon(newIcon);
    }

    ++it;
  }

  thinfo << "Updated " << m_registeredIcons.size() << " dynamic icons safely";
}

void ThemeManager::refreshAllIcons() { updateIconColors(); }

// Color getters
QColor ThemeManager::getCurrentIconColor() const { return m_iconColor; }

QColor ThemeManager::getCurrentHoverColor() const { return m_hoverColor; }

QColor ThemeManager::getCurrentPressedColor() const { return m_pressedColor; }

QColor ThemeManager::getCurrentDisabledColor() const { return m_disabledColor; }

QMap<QString, QString> ThemeManager::reverseThemeMap() const {
  QMap<QString, QString> reversed;
  for (auto it = themeMap.constBegin(); it != themeMap.constEnd(); ++it) {
    reversed[it.value()] = it.key();
  }
  return reversed;
}

QStringList ThemeManager::availableThemes() const { return themeMap.keys(); }

QString ThemeManager::getFileName(const QString& visibleName) {
  return themeMap[visibleName];
}

QString ThemeManager::getVisibleName(const QString& filename) {
  return reversedThemeMap[filename];
}
