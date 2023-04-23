#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
    char *key;
    char *value;
} setting;

bool load_settings(const char *file_name);
void unload_settings();

const char *get_setting_string(const char *key);
int get_setting_int(const char *key);
float get_setting_float(const char *key);

#endif // SETTINGS_H