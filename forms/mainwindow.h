#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QWidget>
#include <QJsonObject>
#include "../skeleton/toastmanager.h"
#include <QToolButton>
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(const QJsonObject& settings, bool updateAvailable, QWidget *parent = nullptr);
  ~MainWindow();

  static QTreeWidget* createTree(QWidget* parent = nullptr) {
    QTreeWidget* tree = new QTreeWidget(parent);
    tree->setRootIsDecorated(false);
    tree->setHeaderHidden(true);
    tree->setItemsExpandable(false);
    tree->setExpandsOnDoubleClick(false);
    tree->setIndentation(0);
    tree->setUniformRowHeights(true);
    tree->setColumnCount(1);
    return tree;
  }

  static bool isRegex(const QString &text) {
    static const QRegularExpression specialChars(R"([.*+?^${}()|\[\]\\])");
    return specialChars.match(text).hasMatch();
  }

 private slots:
  void onAddToFavorites(const QString& itemName);
  void onThemeChanged();

  void on_backBtn_clicked();

  void on_favButton_clicked();

  void on_actionAbout_triggered();

  void on_searchBox_textChanged(const QString &arg1);

 private:
  // initalize side panel
  void initTree();
  QWidget* createGroupEntriesPage(const QString& groupName);
  QWidget* createFavoritesPage(const QJsonArray& favs);
  void slideTransition(QWidget* oldPage, QWidget* newPage, bool reverse = false);
  void loadFavs();

  // search bar with delay
  void loadSearchPage();
  QTimer* searchTimer;
  QString lastSearchText;
  bool isSearching;
  void performSearch();

  // hiddendata: name, visible: tr(name)
  void addItemToTree(QTreeWidget* tree, const QString& name, TreeItemType btntype = TreeItemType::NONE);

  // dynamic icons
  void setupDynamicIcons();
  void refreshIcons();

  void retranslateUi();

  // segments of ctor
  void initializeUi();
  void setupVariables();
  void setupGlobalConnections();

  bool m_isClosing = false;
  bool settingsHasChanges = false;

  int SLIDE_ANIM_DUR;
  static const QString iconsPath;

  // favbtn/rmbtn set stylesheet
  void polishIconButton(QToolButton* toolBtn, bool isRemoveBtn = false) {
    toolBtn->setStyleSheet(QString("QToolButton { background: transparent; border: none; border-radius: 10px; margin: 0px; padding: 0px; }"
      "QToolButton:hover { background-color: rgba(255, %1, 0, 0.3); }").arg(isRemoveBtn ? 0 : 215));
    toolBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolBtn->setAutoRaise(true);
    toolBtn->setFixedSize(btnsqm);
    toolBtn->setContentsMargins(0,0,0,0);
  }

  static constexpr QSize btnsqm = QSize(36, 36); // margined (+8px)
  static constexpr QSize btnsq = QSize(28, 28);

  Ui::MainWindow *ui;
  ToastManager* _toastman;
  QJsonObject m_settings;
  bool m_updateAvailable;
};
#endif  // MAINWINDOW_H
