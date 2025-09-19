#ifndef ICONUTILS_H
#define ICONUTILS_H

#include <QAction>
#include <QColor>
#include <QIcon>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QSize>
#include <QToolButton>
#include <QWidget>

class IconUtils : public QObject {
  Q_OBJECT

 public:
  static IconUtils& instance() {
    static IconUtils _instance;
    return _instance;
  }

  // Batch icon setup functions
  void setupToolbarIcons(QWidget* toolbar,
                         const QMap<QString, QString>& actionIconMap);
  void setupButtonIcons(QWidget* parent,
                        const QMap<QString, QString>& buttonIconMap);

  // Individual icon setup with fallback
  bool setupIcon(QAction* action, const QString& iconPath,
                 const QSize& size = QSize(32, 32));
  bool setupIcon(QPushButton* button, const QString& iconPath,
                 const QSize& size = QSize(32, 32));
  bool setupIcon(QToolButton* button, const QString& iconPath,
                 const QSize& size = QSize(32, 32));

  // Icon validation
  bool validateIconPath(const QString& iconPath) const;
  QStringList findAvailableIcons(const QString& iconDirectory) const;

  // Fallback icon creation
  QIcon createFallbackIcon(const QString& text,
                           const QSize& size = QSize(32, 32),
                           const QColor& bgColor = QColor(100, 100, 100));

 private:
  explicit IconUtils(QObject* parent = nullptr);
  ~IconUtils() = default;
  IconUtils(const IconUtils&) = delete;
  IconUtils& operator=(const IconUtils&) = delete;
};

#endif  // ICONUTILS_H
