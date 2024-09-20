//
// Created by root on 8/27/24.
//

#ifndef OPC_UA_UA_CLIENT_H
#define OPC_UA_UA_CLIENT_H
#include "../opc_ua_plugin.h"
UA_StatusCode ua_client_start(neu_plugin_t *plugin);
int ua_client_stop(neu_plugin_t *plugin);
#endif //OPC_UA_UA_CLIENT_H
