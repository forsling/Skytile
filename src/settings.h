#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef enum {
    SETTING_TYPE_INT,
    SETTING_TYPE_FLOAT,
    SETTING_TYPE_STRING,
    SETTING_TYPE_BOOL
} SettingType;

typedef struct {
    SettingType type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } value;
} Setting;

bool load_settings(const char* file_name, bool is_server);
void write_settings(const char* file_name);
void set_setting(const char* key, SettingType type, const char* value);
void initialize_default_settings();

const char* get_setting_string(const char* key);
int get_setting_int(const char* key);
float get_setting_float(const char* key);
bool get_setting_bool(const char* key);

#endif // SETTINGS_H