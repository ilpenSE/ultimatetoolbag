#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "../skeleton/loggerstream.h"
#include "../skeleton/languagemanager.h"
#include "../etc/instances.h"
#include "../skeleton/thememanager.h"
#include "../skeleton/toastmanager.h"

#include <QTreeWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

const QString MainWindow::iconsPath = ":/assets/icons/";

MainWindow::MainWindow(const QJsonObject& settings, bool updateAvailable, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_settings(settings), m_updateAvailable(updateAvailable),
      isSearching(false) {
  ui->setupUi(this);
  _toastman = new ToastManager(this);

  if (updateAvailable)
    this->setWindowTitle("Ultimate Toolbag (" + tr("update available") + ")");

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

  SLIDE_ANIM_DUR = m_settings["SlideAnimation"].toInt(SettingsManager::defaultSlideAnim);
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
            if (!ui || !ui->toolsStack) return;

            ui->toolsStack->setCurrentWidget(newPage);

            if (group) {
              group->deleteLater();
            }
  });

  group->start(QAbstractAnimation::DeleteWhenStopped);
}
QWidget* MainWindow::createGroupEntriesPage(const QString& groupName) {
  QWidget* page = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(page);

  // create tree widget
  QTreeWidget* tree = createTree(page);
  ui->toolsLabel->setText(tr("Tools") + "/" + tr(qPrintable(groupName)));

  // add entries
  QStringList entries = _groupman.getGroupEntries(groupName);
  for (const QString& entry : std::as_const(entries)) {
    addItemToTree(tree, entry,TreeItemType::FAVORITE);
  }

  layout->addWidget(tree);
  page->setLayout(layout);
  return page;
}

QWidget* MainWindow::createFavoritesPage(const QJsonArray& favs) {
  QWidget* page = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(page);

  // create tree widget
  QTreeWidget* tree = createTree(page);

  // add entries
  for (const auto& fav : std::as_const(favs)) {
    if (!fav.isString()) {
      delete layout;
      delete page;
      delete tree;
      return {};
    }

    QString fstr = fav.toString();
    addItemToTree(tree, fstr, TreeItemType::REMOVE);
  }

  layout->addWidget(tree);
  page->setLayout(layout);
  return page;
}

// BACKEND ADDING
void MainWindow::onAddToFavorites(const QString& itemName) {
  if (_favman.existsFavorite(itemName)) {
    QString errm;
    if (!_favman.removeFavorite(itemName, &errm)) {
      _toastman->showToast(errm, ToastType::ERROR);
      err << errm;
    } else {
      _toastman->showToast(tr("%1 removed from favorites").arg(tr(qPrintable(itemName))), ToastType::SUCCESS);
    }
    return;
  }

  QString errm;
  if (!_favman.addFavorite(itemName, &errm)) {
    QString acterrm = "Error while adding favorite named " + itemName + ": " + errm;
    err << acterrm;
    _toastman->showToast(acterrm, ToastType::ERROR);
    return;
  }
  linfo << "Favorite tool added: " + itemName;
  _toastman->showToast(tr("%1 added to favorites").arg(tr(qPrintable(itemName))), ToastType::SUCCESS);
}

void MainWindow::initTree() {
  QWidget* firstPage = ui->grPage;
  if (firstPage == nullptr) return;

  QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(firstPage->layout());
  if (!layout) {
    layout = new QVBoxLayout(firstPage);
    firstPage->setLayout(layout);
  }

  if (layout->isEmpty()) {
    QTreeWidget* groupTree = createTree(firstPage);
    QStringList groups = _groupman.getGroups();
    for (const QString& gr : std::as_const(groups)) {
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

  // add grPage to stack
  if (ui->toolsStack->indexOf(firstPage) == -1) {
    ui->toolsStack->addWidget(firstPage);
  }

  ui->toolsStack->setCurrentWidget(firstPage);
  firstPage->updateGeometry();
}

void MainWindow::retranslateUi() {
  ui->retranslateUi(this);
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

void MainWindow::onThemeChanged() { refreshIcons(); }

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
  AboutDialog dlg;
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

void MainWindow::addItemToTree(QTreeWidget* tree, const QString& name, TreeItemType btntype) {
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

  QLabel* label = new QLabel(tr(qPrintable(name)));
  label->setStyleSheet("font-weight: 500;");
  label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  itemLayout->addWidget(label);

  if (btntype == TreeItemType::REMOVE) {
    QToolButton* removeBtn = new QToolButton();
    _themesman.setupDynamicButton(
        removeBtn, iconsPath + "cancel.svg", btnsq);
    removeBtn->setText("");
    removeBtn->setToolTip(tr("Remove from favorites"));
    polishIconButton(removeBtn, true);

    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
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
        _toastman->showToast(tr("%1 removed from favorites").arg(tr(qPrintable(name))), ToastType::SUCCESS);
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

    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    favBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    itemLayout->addStretch();
    itemLayout->addWidget(favBtn, 0, Qt::AlignVCenter);

    // fav a tool
    connect(favBtn, &QToolButton::clicked, this, [this, name, favBtn]() {
      onAddToFavorites(name);

      QString favicon = _favman.existsFavorite(name) ? "faved.svg" : "fav.svg";
      _themesman.setupDynamicButton(
          favBtn, iconsPath + favicon, btnsq);

      QTreeWidget* favtree = ui->favPage->findChild<QTreeWidget*>();
      addItemToTree(favtree, name, TreeItemType::REMOVE);
    });

  }

  tree->setItemWidget(item, 0, itemWidget);
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
  QString pattern = lastSearchText;
  bool isreg = isRegex(pattern);
  QRegularExpression re;
  if (isreg) {
    re = QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
    if (!re.isValid()) {
      // Invalid regex, treat as normal text
      isreg = false;
    }
  }

  QJsonArray favs = _favman.getFavorites();
  int resultCount = 0;
  for (auto it = favs.begin(); it != favs.end(); ++it) {
    if (!it->isString()) continue;
    QString val = it->toString("");

    bool matches = false;
    if (!isreg) {
      matches = val.contains(pattern, Qt::CaseInsensitive);
    } else {
      matches = re.match(val).hasMatch();
    }

    if (matches) {
      addItemToTree(tree, val, TreeItemType::FAVORITE);
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
    if (!_forg.saveJson(_forg.settingsFilePath(), m_settings)) {
      fserr << "Settings file cannot be saved!";
    }
  }
  delete ui;
  delete _toastman;
}
