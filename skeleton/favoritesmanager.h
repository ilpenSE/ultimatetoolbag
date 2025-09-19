#ifndef FAVORITESMANAGER_H
#define FAVORITESMANAGER_H

#include <QObject>
#include <QJsonArray>

class FavoritesManager : public QObject {
  Q_OBJECT
 public:
  static FavoritesManager& instance() {
    static FavoritesManager ins;
    return ins;
  }

  bool ensureFavs();
  QJsonArray getFavorites();
  bool addFavorite(const QString& tool, QString* error = nullptr);
  bool removeFavorite(const QString& fav, QString* error = nullptr);
  bool existsFavorite(const QString& tool);

  void saveOnExit();

 private:
  bool createJson();

  QJsonArray favsJson;
  QString jsonPath;
  explicit FavoritesManager(QObject *parent = nullptr);

  // block copy and assign
  FavoritesManager(const FavoritesManager&) = delete;
  FavoritesManager& operator=(const FavoritesManager&) = delete;

  ~FavoritesManager() override = default;
};

#endif  // FAVORITESMANAGER_H
