//
// Created by root on 8/27/24.
//

#ifndef OPC_UA_UA_TAG_HANDLE_H
#define OPC_UA_UA_TAG_HANDLE_H
#include "../opc_ua_plugin.h"
void handle_tag(neu_plugin_t *plugin, neu_datatag_t *tag, neu_plugin_group_t *group);
UA_StatusCode create_and_add_plc_tag(neu_plugin_t *plugin, neu_datatag_t *tag);
#endif //OPC_UA_UA_TAG_HANDLE_H
