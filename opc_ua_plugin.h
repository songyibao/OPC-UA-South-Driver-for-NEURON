//
// Created by root on 8/27/24.
//

#ifndef OPC_UA_OPC_UA_PLUGIN_H
#define OPC_UA_OPC_UA_PLUGIN_H
#include "neuron.h"
#include <open62541/client.h>
typedef struct ua_tag_hash {
    char name[50]; /* key (string is WITHIN the structure) */
    UA_NodeId value;
    UT_hash_handle hh; /* makes this structure hashable */
} ua_tag_hash_t;
struct neu_plugin {
    neu_plugin_common_t common;

    UA_Client *client;
    bool connected;
    bool started;

    char        *host;
    uint32_t    timeout;
    ua_tag_hash_t *ua_tag_hash_table_head;
};
#endif //OPC_UA_OPC_UA_PLUGIN_H
