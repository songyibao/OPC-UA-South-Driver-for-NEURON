//
// Created by root on 8/27/24.
//

#include "ua_tag_handle.h"
#include "../ua_tag_hash/ua_tag_hash.h"
#include <open62541/client_highlevel.h>

void parse_numbers(char *str, int *ns, int *i) {
    // 使用 strtok 来分割字符串
    char *copy = strdup(str); // 创建字符串的副本，因为 strtok 会修改字符串
    char *token;

    // 第一个分隔符 '=' 之后的数字
    token = strtok(copy, "=");
    if (token != NULL) {
        token = strtok(NULL, ";"); // 获取第一个数字字符串 "3"
        if (token != NULL) {
            *ns = (int)strtol(token,NULL,10); // 转换为整数并存储
        }
    }

    // 第二个分隔符 '=' 之后的数字
    token = strtok(NULL, "=");
    if (token != NULL) {
        token = strtok(NULL, ";"); // 获取第二个数字字符串 "1001"
        if (token != NULL) {
            *i = (int)strtol(token,NULL,10); // 转换为整数并存储
        }
    }

    printf("转换成功:ns:%d,i:%d",*ns,*i);
    free(copy); // 释放复制的字符串
}

UA_NodeId get_node_id_from_str(char *str) {
    int ns, i;
    parse_numbers(str, &ns, &i);
    nlog_debug("解析字符串地址成功:%d,%d",ns,i);
    UA_NodeId node_id = UA_NODEID_NUMERIC(ns, i);
    return node_id;
}

UA_StatusCode create_and_add_plc_tag(neu_plugin_t *plugin, neu_datatag_t *tag) {

    int ns, i;
    parse_numbers(tag->address, &ns, &i);
    plog_debug(plugin,"解析字符串地址成功------------------------------>:%d,%d",ns,i);
    UA_NodeId node_id = UA_NODEID_NUMERIC(ns, i);
//    plugin_add_tag(plugin, tag->address, node_id);
    UA_Variant value;
    UA_StatusCode res = UA_Client_readValueAttribute(plugin->client, node_id, &value);
    /* everything OK? */
    if (res != UA_STATUSCODE_BADNODEIDUNKNOWN) {
        plog_debug(plugin,"tag存在，添加该tag");
        plugin_add_tag(plugin, tag->address, node_id);
    }
    return res;
}

int read_tag(neu_plugin_t *plugin, neu_plugin_group_t *group, neu_datatag_t *tag, neu_dvalue_t *dist_value) {
    dist_value->type = tag->type;
    UA_StatusCode res = UA_STATUSCODE_GOOD;
    UA_NodeId node_id;
    ua_tag_hash_t *tag_hash_item = plugin_find_tag(plugin, tag->address);
    if (tag_hash_item) {
        node_id = tag_hash_item->value;
        // 开始连接plc读取数据的时刻
        uint64_t read_tms = neu_time_ms();
        UA_Variant value; /* Variants can hold scalar values and arrays of any type */
        UA_Variant_init(&value);
        /* get the data */
        res = UA_Client_readValueAttribute(plugin->client, node_id, &value);
        if (res != UA_STATUSCODE_GOOD) {
            // 读取失败, 错误处理
            NEU_PLUGIN_UPDATE_METRIC(plugin, NEU_METRIC_LAST_RTT_MS, 9999, NULL);
            plog_error(plugin,
                       "ERROR: Unable to read the data! Got error code %d: %s, deleting tag %s from "
                       "hashtable\n",
                       res, UA_StatusCode_name(res), tag->address);
            dist_value->type = NEU_TYPE_ERROR;
            dist_value->value.i32 = NEU_ERR_PLUGIN_TAG_NOT_READY;
            plugin_del_tag(plugin, tag->address);
            /* Clean up */
            UA_Variant_clear(&value);
            res = -1;
            goto exit;
        }
        plog_debug(plugin,"读取点位:%s,成功",tag->address);

        // 读取成功, 更新读取延迟
        uint64_t read_delay = neu_time_ms() - read_tms;
        NEU_PLUGIN_UPDATE_METRIC(plugin, NEU_METRIC_LAST_RTT_MS, read_delay, NULL);
        // 外层循环中已赋值
        //            dvalue->type      = NEU_TYPE_ERROR;

        if(tag->type>=1 && tag->type<=10 && UA_Variant_hasScalarType(&value, &UA_TYPES[tag->type])==false){
            dist_value->value.i32 = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
            goto exit;
        }else if(tag->type == NEU_TYPE_BOOL && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])==false){
            dist_value->value.i32 = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
            goto exit;
        }
        switch (tag->type) {
            case NEU_TYPE_INT8:
                dist_value->value.i8 = *(UA_SByte *) value.data;
                break;
            case NEU_TYPE_UINT8:
                dist_value->value.u8 = *(UA_Byte *) value.data;
                break;
            case NEU_TYPE_INT16:
                dist_value->value.i16 = *(UA_Int16 *) value.data;
                break;
            case NEU_TYPE_UINT16:
                dist_value->value.u16 = *(UA_UInt16 *) value.data;
                break;
            case NEU_TYPE_INT32:
                dist_value->value.i32 = *(UA_Int32 *) value.data;
                break;
            case NEU_TYPE_UINT32:
                dist_value->value.u32 = *(UA_UInt32 *) value.data;
                break;
            case NEU_TYPE_INT64:
                dist_value->value.i64 = *(UA_Int64 *) value.data;
                break;
            case NEU_TYPE_UINT64:
                dist_value->value.u64 = *(UA_UInt64 *) value.data;
                break;
            case NEU_TYPE_FLOAT:
                dist_value->value.f32 = *(UA_Float *) value.data;
                break;
            case NEU_TYPE_DOUBLE:
                dist_value->value.d64 = *(UA_Double *) value.data;
                break;
            case NEU_TYPE_BIT:
                dist_value->value.u8 = *(UA_Byte *) value.data;
                break;
            case NEU_TYPE_BOOL:
                dist_value->value.boolean = *(UA_Boolean *) value.data; // Assuming BOOL is a single byte
                break;
            case NEU_TYPE_STRING:
                dist_value->value.i32 = NEU_ERR_TAG_TYPE_NOT_SUPPORT;
                break;
            case NEU_TYPE_WORD:
                dist_value->value.u16 = *(UA_UInt16 *) value.data;
                break;
            case NEU_TYPE_DWORD:
                dist_value->value.u32 = *(UA_UInt32 *) value.data;
                break;
            case NEU_TYPE_LWORD:
                dist_value->value.u64 =*(UA_UInt64 *) value.data;
                break;
            default:
                break;
        }
    } else {
        plog_error(plugin, "ua_tag_hash not found,tag->address: %s", tag->address);
        dist_value->type = NEU_TYPE_ERROR;
        dist_value->value.i32 = NEU_ERR_PLUGIN_TAG_NOT_READY;
        plog_debug(plugin, "Trying to create and add tag %s\n", tag->address);
        res = create_and_add_plc_tag(plugin, tag);
        if (res != UA_STATUSCODE_GOOD) {
            plog_debug(plugin, "ERROR %s: Could not create tag!\n", UA_StatusCode_name(res));
        }
        res = -1;
        goto exit;
    }
exit:
    return res==UA_STATUSCODE_GOOD?0:-1;
}

void handle_tag(neu_plugin_t *plugin, neu_datatag_t *tag, neu_plugin_group_t *group) {
    //    plog_debug(plugin,
    //               "tag: %s, address: %s, attribute: %d, type: %d, "
    //               "precision: %d, decimal: %f, description: %s",
    //               tag->name, tag->address, tag->attribute, tag->type, tag->precision, tag->decimal,
    //               tag->description);
    neu_dvalue_t dvalue = {0};
    dvalue.type = tag->type;
    read_tag(plugin, group, tag,&dvalue);
    plugin->common.adapter_callbacks->driver.update(plugin->common.adapter, group->group_name, tag->name, dvalue);
}