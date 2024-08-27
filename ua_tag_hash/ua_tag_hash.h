//
// Created by root on 8/27/24.
//

#ifndef OPC_UA_UA_TAG_HASH_H
#define OPC_UA_UA_TAG_HASH_H
#include "../opc_ua_plugin.h"
ua_tag_hash_t *plugin_find_tag(neu_plugin_t *plugin, char *key);
void        plugin_add_tag(neu_plugin_t *plugin, char *key, UA_NodeId node_id);
void        plugin_del_tag(neu_plugin_t *plugin, char *key);
void        plugin_free_all_tags(neu_plugin_t *plugin);
ua_tag_hash_t *plugin_get_first_tag(neu_plugin_t *plugin);
#endif //OPC_UA_UA_TAG_HASH_H
