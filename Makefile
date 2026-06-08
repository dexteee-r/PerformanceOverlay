# Makefile pour Performance Overlay
# Ce fichier automatise la compilation du programme

# Compilateur et options
CC = gcc
CFLAGS = -Wall -O2 -mwindows -MMD -MP
LIBS = -lgdi32 -luser32 -ladvapi32 -lpsapi -liphlpapi -lws2_32 -lole32 -luuid -lwininet

# Dossiers
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
PLUGIN_DIR = $(SRC_DIR)/plugins
RES_DIR = resources

# Fichiers source
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/performance.c $(SRC_DIR)/config.c $(SRC_DIR)/startup.c \
          $(SRC_DIR)/metric_plugin.c $(SRC_DIR)/config_parser.c $(SRC_DIR)/taskkiller.c \
          $(SRC_DIR)/theme.c $(SRC_DIR)/tray.c $(SRC_DIR)/ui_draw.c \
          $(PLUGIN_DIR)/plugin_cpu.c $(PLUGIN_DIR)/plugin_ram.c $(PLUGIN_DIR)/plugin_disk.c \
          $(PLUGIN_DIR)/plugin_uptime.c $(PLUGIN_DIR)/plugin_process.c \
          $(PLUGIN_DIR)/plugin_network.c $(PLUGIN_DIR)/plugin_datetime.c \
          $(PLUGIN_DIR)/plugin_volume.c $(PLUGIN_DIR)/plugin_gpu.c $(PLUGIN_DIR)/plugin_prayer.c

OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/performance.o $(BUILD_DIR)/config.o $(BUILD_DIR)/startup.o \
          $(BUILD_DIR)/metric_plugin.o $(BUILD_DIR)/config_parser.o $(BUILD_DIR)/taskkiller.o \
          $(BUILD_DIR)/theme.o $(BUILD_DIR)/tray.o $(BUILD_DIR)/ui_draw.o \
          $(BUILD_DIR)/plugin_cpu.o $(BUILD_DIR)/plugin_ram.o $(BUILD_DIR)/plugin_disk.o \
          $(BUILD_DIR)/plugin_uptime.o $(BUILD_DIR)/plugin_process.o \
          $(BUILD_DIR)/plugin_network.o $(BUILD_DIR)/plugin_datetime.o \
          $(BUILD_DIR)/plugin_volume.o $(BUILD_DIR)/plugin_gpu.o $(BUILD_DIR)/plugin_prayer.o \
          $(BUILD_DIR)/resources.o

# Fichiers de dépendances auto-générés par GCC (-MMD -MP)
DEPS = $(OBJECTS:.o=.d)

# Nom de l'exécutable
TARGET = PerformanceOverlay_v2.exe

# Règle par défaut
all: $(BUILD_DIR) $(TARGET)

# Créer le dossier build s'il n'existe pas
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Compilation de l'exécutable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo Compilation terminee! Executable: $(TARGET)

# Compilation de main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de performance.c
$(BUILD_DIR)/performance.o: $(SRC_DIR)/performance.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de config.c
$(BUILD_DIR)/config.o: $(SRC_DIR)/config.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de startup.c
$(BUILD_DIR)/startup.o: $(SRC_DIR)/startup.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de metric_plugin.c
$(BUILD_DIR)/metric_plugin.o: $(SRC_DIR)/metric_plugin.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de config_parser.c
$(BUILD_DIR)/config_parser.o: $(SRC_DIR)/config_parser.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de taskkiller.c
$(BUILD_DIR)/taskkiller.o: $(SRC_DIR)/taskkiller.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de theme.c
$(BUILD_DIR)/theme.o: $(SRC_DIR)/theme.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de tray.c
$(BUILD_DIR)/tray.o: $(SRC_DIR)/tray.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de ui_draw.c
$(BUILD_DIR)/ui_draw.o: $(SRC_DIR)/ui_draw.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation des plugins
$(BUILD_DIR)/plugin_cpu.o: $(PLUGIN_DIR)/plugin_cpu.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_ram.o: $(PLUGIN_DIR)/plugin_ram.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_disk.o: $(PLUGIN_DIR)/plugin_disk.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_uptime.o: $(PLUGIN_DIR)/plugin_uptime.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_process.o: $(PLUGIN_DIR)/plugin_process.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_network.o: $(PLUGIN_DIR)/plugin_network.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_datetime.o: $(PLUGIN_DIR)/plugin_datetime.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_volume.o: $(PLUGIN_DIR)/plugin_volume.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_gpu.o: $(PLUGIN_DIR)/plugin_gpu.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

$(BUILD_DIR)/plugin_prayer.o: $(PLUGIN_DIR)/plugin_prayer.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation des ressources
$(BUILD_DIR)/resources.o: $(RES_DIR)/resources.rc $(RES_DIR)/icon.ico
	windres $(RES_DIR)/resources.rc -o $@

# Nettoyer les fichiers compilés
clean:
	@if exist "$(BUILD_DIR)" rd /s /q "$(BUILD_DIR)"
	@if exist "$(TARGET)" del /q "$(TARGET)"
	@echo Nettoyage termine!

# Recompiler complètement
rebuild: clean all

# Lancer le programme après compilation
run: all
	@echo Lancement du programme...
	@$(TARGET)

.PHONY: all clean rebuild run

# Inclusion des fichiers de dépendances (générés automatiquement par GCC)
-include $(DEPS)
