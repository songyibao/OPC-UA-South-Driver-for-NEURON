//
// Created by root on 8/27/24.
//

#include "ua_client.h"
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
UA_StatusCode ua_client_start(neu_plugin_t *plugin){
    if(plugin->client != NULL){
        return -2;
    }

    // 创建新的 OPC UA 客户端
    plugin->client = UA_Client_new();
    UA_ClientConfig *config = UA_Client_getConfig(plugin->client);

    // 设置默认配置
    UA_ClientConfig_setDefault(config);

    // 设置连接超时时间，例如设置为（500毫秒）
    config->timeout = plugin->timeout;
    plog_debug(plugin,"连接超时时间：%d",config->timeout);

    // 尝试连接到服务器
    UA_StatusCode retval = UA_Client_connect(plugin->client, plugin->host);
    if(retval != UA_STATUSCODE_GOOD) {
        printf("错误：%s", UA_StatusCode_name(retval));
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s", UA_StatusCode_name(retval));
        UA_Client_delete(plugin->client);
        plugin->client = NULL;
        return retval;
    }

    plugin->common.link_state = NEU_NODE_LINK_STATE_CONNECTED;
    plugin->connected = true;
    plog_info(plugin, "创建 OPC UA client 成功");

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