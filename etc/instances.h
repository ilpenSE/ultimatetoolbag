#ifndef INSTANCES_H
#define INSTANCES_H

#include "../skeleton/fileorganizer.h"
#include "../skeleton/languagemanager.h"
#include "../skeleton/settingsmanager.h"
#include "../skeleton/thememanager.h"
#include "../skeleton/assetvalidator.h"
#include "../skeleton/groupmanager.h"
#include "../skeleton/versionmanager.h"
#include "../skeleton/favoritesmanager.h"
#include "../lib/apimanager.h"
#include "../lib/toolwidgetmanager.h"
#include "../lib/ecbrates.h"

#define _favman FavoritesManager::instance()
#define _verman VersionManager::instance()
#define _groupman GroupManager::instance()
#define _themesman ThemeManager::instance()
#define _langman LanguageManager::instance()
#define _forg FileOrganizer::instance()
#define _settingsman SettingsManager::instance()
#define _avalidator AssetValidator::instance()
#define _apiman APIManager::instance()
#define _toolwman ToolWidgetManager::instance()
#define _ecbrates ECBRates::instance()

#endif  // INSTANCES_H
