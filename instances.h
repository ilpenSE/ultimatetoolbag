#ifndef INSTANCES_H
#define INSTANCES_H

#include "thememanager.h"
#include "languagemanager.h"
#include "settingsmanager.h"
#include "appdatamanager.h"
#include "etagcontroller.h"

inline ThemeManager& _themesman() { return ThemeManager::instance(); }
inline LanguageManager& _langman() { return LanguageManager::instance(); }
inline AppDataManager& _appdataman() { return AppDataManager::instance(); }
inline SettingsManager& _settingsman() { return SettingsManager::instance(); }
inline EtagController& _etagcon() { return EtagController::instance(); }
#endif  // INSTANCES_H
