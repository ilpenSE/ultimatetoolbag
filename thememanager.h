#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QAction>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QPointer>
#include <QPushButton>
#include <QStandardPaths>
#include <QToolButton>

class ThemeManager : public QObject {
  Q_OBJECT
 public:
  static ThemeManager& instance() {
    static ThemeManager _instance;
    return _instance;
  }

  // functions
  bool applyTheme(const QString& visibleName);
  QStringList availableThemes() const;
  QString themesDirPath() const;
  QString themesJsonPath() const;
  void loadThemesFromJson();
  QMap<QString, QString> reverseThemeMap() const;
  QString getVisibleName(const QString& filename);
  QString getFileName(const QString& visibleName);
  QPixmap coloredPixmap(const QString& svgPath, const QColor& color,
                        const QSize& size = QSize(32, 32));
  void resolveVars(QString* qssContent) const;

  // New dynamic icon functions
  QIcon createDynamicIcon(const QString& svgPath,
                          const QSize& size = QSize(32, 32));
  void setupDynamicAction(QAction* action, const QString& svgPath,
                          const QSize& size = QSize(32, 32));
  void setupDynamicButton(QPushButton* button, const QString& svgPath,
                          const QSize& size = QSize(32, 32));
  void setupDynamicButton(QToolButton* button, const QString& svgPath,
                          const QSize& size);
  void refreshAllIcons();
  void setIconColors(const QColor& normal, const QColor& hover,
                     const QColor& pressed, const QColor& disabled);

  // Theme color getters
  QColor getCurrentIconColor() const;
  QColor getCurrentHoverColor() const;
  QColor getCurrentPressedColor() const;
  QColor getCurrentDisabledColor() const;

 signals:
  void themeChanged();

 private slots:
  void updateIconColors();

 private:
  QString readQssFile(const QString& filePath) const;
  explicit ThemeManager(QObject* parent = nullptr);
  ~ThemeManager() = default;

  ThemeManager(const ThemeManager&) = delete;
  ThemeManager& operator=(const ThemeManager&) = delete;

  // Icon management
  struct IconData {
    QString svgPath;
    QSize size;
    QPointer<QObject> widget;
  };

  QMap<QObject*, IconData> m_registeredIcons;
  void parseThemeColors(const QString& qssContent);

  // Current theme colors
  QColor m_iconColor = QColor(70, 70, 70);
  QColor m_hoverColor = QColor(50, 50, 50);
  QColor m_pressedColor = QColor(30, 30, 30);
  QColor m_disabledColor = QColor(150, 150, 150);
};

#endif  // THEMEMANAGER_H
