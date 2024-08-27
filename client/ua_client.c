//
// Created by root on 8/27/24.
//

#include "ua_client.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
int ua_client_start(neu_plugin_t *plugin){
    plugin->client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(plugin->client));
    UA_StatusCode retval = UA_Client_connect(plugin->client, plugin->host);
//    UA_StatusCode retval = UA_Client_connectUsername(client, "opc.tcp://10.142.0.111:4840/OPCUA/SimulationServer","seclab",".999999999a");
    if(retval != UA_STATUSCODE_GOOD) {
        printf("错误：%s",UA_StatusCode_name(retval));
        UA_LOG_ERROR(UA_Log_Stdout,UA_LOGCATEGORY_USERLAND, "%s",UA_StatusCode_name(retval));
        UA_Client_delete(plugin->client);
        return (int)retval;
    }
    plugin->common.link_state = NEU_NODE_LINK_STATE_CONNECTED;
    plugin->connected = true;
    plog_info(plugin,"创建 OPC UA client成功");
    return UA_STATUSCODE_GOOD;
}
int ua_client_stop(neu_plugin_t *plugin){
    if(plugin->client==NULL){
        return -1;
    }
    UA_Client_delete(plugin->client); /* Disconnects the client internally */
    plugin->client = NULL;
    plog_info(plugin,"销毁 OPC UA client成功");
    plugin->common.link_state = NEU_NODE_LINK_STATE_DISCONNECTED;
    plugin->connected = false;
    return 0;
}