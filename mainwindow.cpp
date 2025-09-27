#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "skeleton/loggerstream.h"
#include "skeleton/languagemanager.h"
#include "etc/instances.h"
#include "etc/enums.h"
#include "skeleton/thememanager.h"
#include "skeleton/toastmanager.h"
#include "lib/jsonworker.h"
#include "lib/cryptomanager.h"

#include <QTreeWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QDockWidget>

const QString MainWindow::iconsPath = ":/assets/icons/";

MainWindow::MainWindow(const QJsonObject& settings, bool updateAvailable, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_settings(settings), m_updateAvailable(updateAvailable),
      isSearching(false) {
  ui->setupUi(this);
  _toastman = new ToastManager(this);

  if (updateAvailable) {
    this->setWindowTitle("Ultimate Toolbag (" + tr("update available") + ")");
    _toastman->showToast(tr("update available"));
  }

  setupGlobalConnections();

  // wait until ui is ready
  QTimer::singleShot(30, this, [this]() {
    if (!m_isClosing) {
      setupVariables();
      initializeUi();
    }
  });
}

// CTOR CONNECTIONS HERE:
void MainWindow::setupGlobalConnections() {
  // connect to language changes
  connect(&LanguageManager::instance(), &LanguageManager::languageChanged, this,
          &MainWindow::retranslateUi);

  // Connect to theme changes
  connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this,
          &MainWindow::onThemeChanged);
}

// GLOBAL VARIABLE INIT HERE:
void MainWindow::setupVariables() {
  // Search timer setup
  searchTimer = new QTimer(this);
  searchTimer->setSingleShot(true);
  searchTimer->setInterval(200); // debounce delay
  connect(searchTimer, &QTimer::timeout, this, &MainWindow::performSearch);

  // slide animation duration
  SLIDE_ANIM_DUR = m_settings["SlideAnimation"].toInt(SettingsManager::defaultSlideAnim);

  // translated tools
  translatedTools.clear();
  translatedTools.reserve(_groupman.toolKeys.size());
  for (const QString &tool : std::as_const(_groupman.toolKeys)) {
    QString translated = sftr(tool);
    // Normalize Unicode ve UTF-8 encoding
    translated = translated.normalized(QString::NormalizationForm_C);
    translatedTools.append(QString::fromUtf8(translated.toUtf8()));
  }

  m_groups = _groupman.getGroups();
}

// UI MARKUP CODES HERE:
void MainWindow::initializeUi() {
  setupDynamicIcons();
  initTree();
  loadFavs();
  loadSearchPage();

  // fix tools label
  ui->toolsLabel->setStyleSheet("font-size:16pt;font-weight:bold;");

  ui->backBtn->setVisible(false);
}

void MainWindow::slideTransition(QWidget* oldPage, QWidget* newPage, bool reverse) {
  if (!oldPage || !newPage) return;

  if (ui->toolsStack->indexOf(newPage) == -1) {
    ui->toolsStack->addWidget(newPage);
  }

  QRect geom = oldPage->geometry();
  if (!geom.isValid() || geom.width() == 0 || geom.height() == 0) {
    geom = ui->toolsStack->geometry();
    geom.moveTo(0, 0);
  }

  int w = geom.width();

  // set newPage start pos
  if (reverse) {
    newPage->move(geom.x() - w, geom.y());
    newPage->resize(w, geom.height());
  } else {
    newPage->move(geom.x() + w, geom.y());
    newPage->resize(w, geom.height());
  }

  // show widget
  newPage->show();
  newPage->raise();

  // anims
  QPropertyAnimation* animOld = new QPropertyAnimation(oldPage, "geometry", this);
  animOld->setDuration(SLIDE_ANIM_DUR);
  animOld->setStartValue(oldPage->geometry());
  animOld->setEasingCurve(QEasingCurve::InOutQuad);
  if (reverse) {
    animOld->setEndValue(QRect(geom.x() + w, geom.y(), w, geom.height()));
  } else {
    animOld->setEndValue(QRect(geom.x() - w, geom.y(), w, geom.height()));
  }

  QPropertyAnimation* animNew = new QPropertyAnimation(newPage, "geometry", this);
  animNew->setDuration(SLIDE_ANIM_DUR);
  animNew->setStartValue(newPage->geometry());
  animNew->setEndValue(geom);
  animNew->setEasingCurve(QEasingCurve::InOutQuad);

  QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
  group->addAnimation(animOld);
  group->addAnimation(animNew);

  // safe lambda
  connect(group, &QParallelAnimationGroup::finished, this,
          [this, newPage, oldPage, reverse, group]() {
            // Null control
            if (!ui->toolsStack) return;

            ui->toolsStack->setCurrentWidget(newPage);

            if (group) {
              group->deleteLater();
            }
  });

  group->start(QAbstractAnimation::DeleteWhenStopped);
}

QWidget* MainWindow::createGroupEntriesPage(const QString& groupName) {
  ui->toolsLabel->setText(tr("Tools") + "/" + sftr(groupName));

  // Cache lookup
  if (pageCache.contains(groupName)) {
    // update LRU
    pageOrder.removeAll(groupName);
    pageOrder.prepend(groupName);
    return pageCache[groupName];
  }

  // if not exist on cache, create one.
  QWidget* page = new QWidget(ui->toolsStack);
  QVBoxLayout* layout = new QVBoxLayout(page);

  // create tree widget
  QTreeWidget* tree = createTree(page);

  // add entries
  QStringList entries = _groupman.getGroupEntries(groupName);
  for (const QString& entry : std::as_const(entries)) {
    QTreeWidgetItem* it = addItemToTree(tree, entry, TreeItemType::FAVORITE);
    connectItemClicks(it, tree);
  }

  layout->addWidget(tree);
  page->setLayout(layout);

  // add to cache
  pageCache[groupName] = page;
  pageOrder.prepend(groupName);

  // Limit control
  if (pageOrder.size() > maxCacheSize) {
    QString last = pageOrder.takeLast();
    QWidget* oldPage = pageCache.take(last);
    if (oldPage) {
      ui->toolsStack->removeWidget(oldPage);
      oldPage->deleteLater();
    }
  }

  return page;
}

QWidget* MainWindow::createFavoritesPage(const QJsonArray& favs) {
  QWidget* page = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(page);

  // create tree widget
  QTreeWidget* tree = createTree(page);

  // add entries
  for (const auto& fav : std::as_const(favs)) {
    QString fstr = fav.toString();
    QTreeWidgetItem* it = addItemToTree(tree, fstr, TreeItemType::REMOVE);
    favItems[fstr] = it;

    connectItemClicks(it, tree);
  }

  layout->addWidget(tree);
  page->setLayout(layout);

  return page;
}


#include "lib/generictoolwidget.h"
void MainWindow::openToolTab(const QString& toolName) {
  // Önce kontrol et, zaten açık mı
  QList<QDockWidget*> dockables = findChildren<QDockWidget*>();
  for (QDockWidget* dock : std::as_const(dockables)) {
    if (dock->windowTitle() == toolName) {
      dock->raise();
      return;
    }
  }

  QDockWidget* dock = new QDockWidget(toolName, this);
  dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

  GenericToolWidget* w = new GenericToolWidget();
  w->showWidget(toolName);

  dock->setWidget(w);
  addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::connectItemClicks(QTreeWidgetItem* it, QTreeWidget* tree) {
  connect(tree, &QTreeWidget::itemClicked, this,
          [this, it](QTreeWidgetItem* clickedItem, int column) {
            if (clickedItem != it) return;
            QString toolName = clickedItem->data(0, Qt::UserRole).toString();
            if (!toolName.isEmpty())
              openToolTab(toolName);
          }
  );
}

// BACKEND ADDING (if it added returns true)
bool MainWindow::onAddToFavorites(const QString& itemName) {
  if (_favman.existsFavorite(itemName)) {
    QString errm;
    if (!_favman.removeFavorite(itemName, &errm)) {
      _toastman->showToast(errm, ToastType::ERROR);
      err << errm;
    } else {
      _toastman->showToast(tr("%1 removed from favorites").arg(sftr(itemName)), ToastType::SUCCESS);
    }
    return false;
  }

  QString errm;
  if (!_favman.addFavorite(itemName, &errm)) {
    QString acterrm = "Error while adding favorite named " + itemName + ": " + errm;
    err << acterrm;
    _toastman->showToast(acterrm, ToastType::ERROR);
    return false;
  }
  linfo << "Favorite tool added: " + itemName;
  _toastman->showToast(tr("%1 added to favorites").arg(sftr(itemName)), ToastType::SUCCESS);
  return true;
}

void MainWindow::initTree() {
  QWidget* firstPage = ui->grPage;
  if (!firstPage) return;

  QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(firstPage->layout());
  if (!layout) {
    layout = new QVBoxLayout(firstPage);
    firstPage->setLayout(layout);
  }

  if (layout->isEmpty()) {
    QTreeWidget* groupTree = createTree(firstPage);
    for (const QString& gr : std::as_const(m_groups)) {
      addItemToTree(groupTree, gr);
    }
    layout->addWidget(groupTree);

    connect(groupTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem* item, int) {
      QString group = item->data(0, Qt::UserRole).toString();
      QWidget* oldPage = ui->toolsStack->currentWidget();
      QWidget* newPage = createGroupEntriesPage(group);
      ui->backBtn->setVisible(true);
      slideTransition(oldPage, newPage, false);
    });
  }

  ui->toolsStack->setCurrentWidget(firstPage);
  firstPage->updateGeometry();
}

void MainWindow::retranslateUi() {
  ui->retranslateUi(this);

  // update translated tools
  translatedTools.clear();
  translatedTools.reserve(_groupman.toolKeys.size());
  for (const QString &tool : std::as_const(_groupman.toolKeys)) {
    QString translated = sftr(tool);
    // Normalize Unicode ve UTF-8 encoding
    translated = translated.normalized(QString::NormalizationForm_C);
    translatedTools.append(QString::fromUtf8(translated.toUtf8()));
  }

  // update grPage
  retranslatePage(ui->grPage, m_groups);

  // update fav items -> tool: item
  retranslatePage(ui->favPage, QStringList(favItems.keys()));

  // update page caches
  retranslatePageCache();
}

void MainWindow::retranslatePage(QWidget* page, const QStringList& keys) {
  QTreeWidget* tree = page->findChild<QTreeWidget*>();
  if (!tree) return;

  for (const QString& key : keys) {
    retranslateItem(tree, key);
  }
}
void MainWindow::retranslatePageCache() {
  for (auto it = pageCache.begin(); it != pageCache.end(); ++it) {
    QTreeWidget* ptree = it.value()->findChild<QTreeWidget*>();
    if (!ptree) continue;

    // tüm top-level item'ları dolaş
    for (int i = 0; i < ptree->topLevelItemCount(); ++i) {
      QTreeWidgetItem* item = ptree->topLevelItem(i);
      if (!item) continue;

      QString key = item->data(0, Qt::UserRole).toString();
      if (!key.isEmpty()) {
        retranslateItem(ptree, key);
      }
    }
  }
}

void MainWindow::setupDynamicIcons() {
  // QActions
  // settings
  if (ui->actionSettings) {
    _themesman.setupDynamicAction(
        ui->actionSettings, iconsPath + "settings.svg", QSize(24, 24));
  }
  // about
  if (ui->actionAbout) {
    _themesman.setupDynamicAction(
        ui->actionAbout, iconsPath + "info.svg", QSize(24, 24));
  }
  // manage groups
  if (ui->actionManageGroups) {
    _themesman.setupDynamicAction(
        ui->actionManageGroups, iconsPath + "edit.svg", QSize(24, 24));
  }
  // account
  if (ui->actionAccount) {
    _themesman.setupDynamicAction(
        ui->actionAccount, iconsPath + "account.svg", QSize(24,24));
  }
  // Buttons
  // fav btn
  if (ui->favButton) {
    _themesman.setupDynamicButton(
        ui->favButton, iconsPath + "faved.svg", QSize(24, 24));
  }
  // back btn
  if (ui->backBtn) {
    _themesman.setupDynamicButton(
        ui->backBtn, iconsPath + "back.svg", QSize(16,16));
  }
  thinfo << "Dynamic icons setup completed";
}

void MainWindow::onThemeChanged() {
  refreshIcons();
}

void MainWindow::refreshIcons() {
  // Force refresh all icons if needed
  _themesman.refreshAllIcons();
}

void MainWindow::on_backBtn_clicked() {
  if (!m_isClosing && ui && ui->toolsStack && ui->grPage) {
    QWidget* currentPage = ui->toolsStack->currentWidget();
    if (currentPage && currentPage != ui->grPage) {
      // Stop any pending search
      searchTimer->stop();
      isSearching = false;

      // Clear search box if coming from search
      if (currentPage == ui->searchPage) {
        ui->searchBox->clear();
      }

      // grPage must be on the stack, check out
      if (ui->toolsStack->indexOf(ui->grPage) == -1) {
        crit << "(!!! SEGFAULT RISK !!!) grPage not in the stack!";
        ui->toolsStack->addWidget(ui->grPage);
      }
      slideTransition(currentPage, ui->grPage, true);
      ui->toolsLabel->setText(tr("Tools"));
      ui->backBtn->setVisible(false);
    }
  }
}

void MainWindow::loadFavs() {
  ui->favPage = createFavoritesPage(_favman.getFavorites());
}

void MainWindow::on_favButton_clicked()
{
  if (!ui->toolsStack->currentWidget() || !ui->favPage) return;

  if (ui->toolsStack->currentWidget() == ui->favPage) return;

  QWidget* oldPage = ui->toolsStack->currentWidget();
  ui->backBtn->setVisible(true);
  ui->toolsLabel->setText(tr("Tools") + "/" + tr("Favorite tools"));
  slideTransition(oldPage, ui->favPage, false);
}

#include "aboutdialog.h"
void MainWindow::on_actionAbout_triggered()
{
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::loadSearchPage() {
  QWidget* page = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(page);

  // create tree widget
  QTreeWidget* tree = createTree(page);

  layout->addWidget(tree);
  page->setLayout(layout);

  ui->searchPage = page;
}

QTreeWidgetItem* MainWindow::addItemToTree(QTreeWidget* tree, const QString& name, TreeItemType btntype) {
  QTreeWidgetItem* item = new QTreeWidgetItem(tree);
  item->setText(0, "");
  item->setData(0, Qt::UserRole, name);

  // Custom widget
  QWidget* itemWidget = new QWidget();
  itemWidget->setMinimumHeight(30);
  itemWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
  itemLayout->setContentsMargins(12, 2, 12, 2);
  itemLayout->setSpacing(8);

  QLabel* label = new QLabel(sftr(name));
  label->setStyleSheet("font-weight: 500;");
  label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  itemLayout->addWidget(label);

  if (btntype == TreeItemType::REMOVE) {
    QToolButton* removeBtn = new QToolButton();
    _themesman.setupDynamicButton(
        removeBtn, iconsPath + "cancel.svg", btnsq);
    removeBtn->setText("");
    removeBtn->setToolTip(tr("Remove from favorites"));
    polishIconButton(removeBtn, true);

    removeBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    itemLayout->addStretch();
    itemLayout->addWidget(removeBtn, 0, Qt::AlignVCenter);

    // remove a favorite tool
    connect(removeBtn, &QToolButton::clicked, this, [this, name, item]() {
      QString errm;
      if (!_favman.removeFavorite(name, &errm)) {
        _toastman->showToast(errm, ToastType::ERROR);
        err << errm;
      } else {
        _toastman->showToast(tr("%1 removed from favorites").arg(sftr(name)), ToastType::SUCCESS);
      }
      delete item;
    });
  }

  if (btntype == TreeItemType::FAVORITE) {
    QToolButton* favBtn = new QToolButton();
    QString favicon = _favman.existsFavorite(name) ? "faved.svg" : "fav.svg";
    _themesman.setupDynamicButton(
        favBtn, iconsPath + favicon, btnsq);
    favBtn->setText("");
    favBtn->setToolTip(tr("Add to favorites"));
    polishIconButton(favBtn);

    favBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    itemLayout->addStretch();
    itemLayout->addWidget(favBtn, 0, Qt::AlignVCenter);

    // fav a tool
    connect(favBtn, &QToolButton::clicked, this, [this, name, favBtn]() {
      bool isadded = onAddToFavorites(name);

      QString favicon = isadded ? "faved.svg" : "fav.svg";
      _themesman.setupDynamicButton(
          favBtn, iconsPath + favicon, btnsq);

      QTreeWidget* favtree = ui->favPage->findChild<QTreeWidget*>();
      if (isadded) {
        favItems[name] = addItemToTree(favtree, name, TreeItemType::REMOVE);
      } else {
        if (favItems.contains(name)) {
          QTreeWidgetItem* item = favItems.take(name);
          delete favtree->takeTopLevelItem(favtree->indexOfTopLevelItem(item));
        }
      }
    });

  }

  tree->setItemWidget(item, 0, itemWidget);
  return item;
}

void MainWindow::retranslateItem(QTreeWidget* tree, const QString& keyName) {
  if (!tree) return;

  for (int i = 0; i < tree->topLevelItemCount(); ++i) {
    QTreeWidgetItem* item = tree->topLevelItem(i);
    if (!item) continue;

    QString key = item->data(0, Qt::UserRole).toString();
    if (key == keyName) {
      QWidget* w = tree->itemWidget(item, 0);
      if (w) {
        QLabel* lbl = w->findChild<QLabel*>();
        if (lbl) {
          lbl->setText(sftr(keyName));
        }
      }
      return;
    }
  }
}

void MainWindow::performSearch() {
  if (!ui->searchPage || !ui->toolsStack->currentWidget()) return;

  QTreeWidget* tree = ui->searchPage->findChild<QTreeWidget*>();
  if (!tree) return;

  // Clear all items
  while (tree->topLevelItemCount() > 0) {
    delete tree->takeTopLevelItem(0);
  }

  if (lastSearchText.isEmpty()) {
    if (isSearching) {
      isSearching = false;
      on_backBtn_clicked();
    }
    return;
  }

  // If we're not already in search mode, switch to search page
  if (!isSearching || ui->toolsStack->currentWidget() != ui->searchPage) {
    isSearching = true;
    QWidget* oldPage = ui->toolsStack->currentWidget();

    // Ensure search page is added to stack
    if (ui->toolsStack->indexOf(ui->searchPage) == -1) {
      ui->toolsStack->addWidget(ui->searchPage);
    }

    slideTransition(oldPage, ui->searchPage, false);
    ui->backBtn->show();
    ui->toolsLabel->setText(tr("Search"));
  }

  // Perform the actual search
  QString pattern = lastSearchText.normalized(QString::NormalizationForm_C);
  bool isreg = isRegex(pattern);
  QRegularExpression re;
  if (isreg) {
    re = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    if (!re.isValid()) {
      // Invalid regex, treat as normal text
      isreg = false;
    }
  }

  int resultCount = 0;
  for (int i = 0; i < translatedTools.size(); ++i) {
    const QString& translatedTool = translatedTools.at(i);
    bool matches = false;
    if (!isreg) {
      matches = translatedTool.contains(pattern, Qt::CaseInsensitive);
    } else {
      matches = re.match(translatedTool).hasMatch();
    }

    if (matches) {
      QTreeWidgetItem* it = addItemToTree(tree, _groupman.toolKeys.values().at(i), TreeItemType::FAVORITE);
      connectItemClicks(it, tree);
      resultCount += 1;
    }
  }

  if (resultCount == 0) {
    _toastman->showToast(tr("No search result found"));
  }
}
void MainWindow::on_searchBox_textChanged(const QString &arg1) {
  lastSearchText = arg1;

  // Stop any pending search
  searchTimer->stop();

  if (arg1.isEmpty()) {
    // Immediate action for empty text
    if (isSearching) {
      isSearching = false;
      on_backBtn_clicked();
    }
    return;
  }

  // Start debounced search
  searchTimer->start();
}

// DESTRUCTOR
MainWindow::~MainWindow() {
  m_isClosing = true;

  CryptoManager::cleanupCryptoManager();

  if (ui && ui->toolsStack) {
    for (int i = 0; i < ui->toolsStack->count(); ++i) {
      QWidget* widget = ui->toolsStack->widget(i);
      if (widget) {
        // Widget'taki animasyonları durdur
        widget->setParent(nullptr);
      }
    }
  }

  // Stop search timer
  if (searchTimer) {
    searchTimer->stop();
  }

  // save settings to file
  if (settingsHasChanges) {
    if (!JSONWorker::saveJson(_forg.appFolderPath() + "/settings.json", m_settings)) {
      fserr << "Settings file cannot be saved!";
    }
  }
  delete ui;
  delete _toastman;
}

#include "settingswin.h"
void MainWindow::on_actionSettings_triggered()
{
  QJsonObject oldSettings = m_settings;
  SettingsWin settingsDialog(&m_settings, this);

  QDialog::DialogCode result = static_cast<QDialog::DialogCode>(settingsDialog.exec());

  if (result == QDialog::DialogCode::Accepted) {
    // new vars
    QString newtheme = m_settings["Theme"].toString();
    QString newlang = m_settings["Language"].toString();
    int newslidea = m_settings["SlideAnimation"].toInt();

    // check if theme changed
    if (oldSettings["Theme"].toString() != newtheme) {
      settingsHasChanges = true;
      QString visname = sftr(_themesman.getVisibleName(newtheme));
      if (!_themesman.applyTheme(newtheme)) {
        _toastman->showToast(tr("Theme %1 couldn't be loaded! Changing it to Fluent Dark...").arg(visname), ToastType::WARNING);
        // fallback: retry as dark
        if (!_themesman.applyTheme("dark")) {
          thcrit << "Fallback theme couldn't be loaded after settings tab!";
          _toastman->showToast(tr("Fluent Dark theme couldn't be loaded."), ToastType::ERROR);
          return;
        }
        _toastman->showToast(tr("Successfully changed theme to Fluent Dark"), ToastType::SUCCESS);
      }
      _toastman->showToast(tr("Successfully changed theme to %1").arg(visname), ToastType::SUCCESS);
    }

    // check if language changed
    if (oldSettings["Language"].toString() != newlang) {
      settingsHasChanges = true;
      QString visname = getLanguageStr(newlang);
      if (!_langman.changeLanguage(newlang)) {
        _toastman->showToast(tr("Language %1 couldn't be loaded! Changing it to English...").arg(visname), ToastType::WARNING);
        // fallback: retry as english
        if (!_langman.changeLanguage("en_US")) {
          thcrit << "Fallback language couldn't be loaded after settings tab!";
          _toastman->showToast(tr("English couldn't be loaded."), ToastType::ERROR);
          return;
        }
        _toastman->showToast(tr("Successfully changed language to English"), ToastType::SUCCESS);
      }
      _toastman->showToast(tr("Successfully changed language to %1").arg(visname), ToastType::SUCCESS);
    }

    // check if slide animation changed
    if (oldSettings["SlideAnimation"].toInt() != newslidea) {
      settingsHasChanges = true;

      SLIDE_ANIM_DUR = newslidea;
    }

  }
}
