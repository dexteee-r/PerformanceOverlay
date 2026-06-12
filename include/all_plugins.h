/*
 * all_plugins.h
 * Déclarations de tous les plugins disponibles
 */

#ifndef ALL_PLUGINS_H
#define ALL_PLUGINS_H

#include "metric_plugin.h"

// Déclarations externes des plugins
extern MetricPlugin CPUPlugin;
extern MetricPlugin RAMPlugin;
extern MetricPlugin DiskPlugin;
extern MetricPlugin UptimePlugin;
extern MetricPlugin ProcessPlugin;
extern MetricPlugin NetworkPlugin;
extern MetricPlugin DateTimePlugin;
extern MetricPlugin VolumePlugin;
extern MetricPlugin GPUPlugin;
extern MetricPlugin PrayerPlugin;
extern MetricPlugin ClaudeUsagePlugin;

// Configuration des prieres (ancienne fonction pour compatibilite)
void SetPrayerTimes(const char* fajr, const char* dhuhr, const char* asr,
                    const char* maghrib, const char* isha, BOOL enabled);

// Configuration complete des prieres avec API
void SetPrayerConfig(const char* city, const char* country, int method,
                     BOOL useApi, BOOL enabled,
                     const char* fajr, const char* dhuhr, const char* asr,
                     const char* maghrib, const char* isha);

#endif // ALL_PLUGINS_H
