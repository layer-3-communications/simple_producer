#ifndef CONFIG_H__
#define CONFIG_H__

#include <stdio.h>
#include <stdbool.h>

#include <yaml.h>

#include "types.h"

config_t *load_config(const char *configfile);
void free_config(config_t* config);
void valid_config(config_t* config);

#endif /* CONFIG_H__ */
