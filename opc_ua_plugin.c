//
// Created by root on 8/27/24.
//

#include "opc_ua_plugin.h"
#include <stdlib.h>

#include "neuron.h"

#include "errcodes.h"
#include "ua_tag_handle/ua_tag_handle.h"
#include "client/ua_client.h"
#include "ua_tag_hash/ua_tag_hash.h"
static neu_plugin_t *driver_open(void);

static int driver_close(neu_plugin_t *plugin);
static int driver_init(neu_plugin_t *plugin, bool load);
static int driver_uninit(neu_plugin_t *plugin);
static int driver_start(neu_plugin_t *plugin);
static int driver_stop(neu_plugin_t *plugin);
static int driver_config(neu_plugin_t *plugin, const char *config);
static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head, void *data);

static int driver_tag_validator(const neu_datatag_t *tag);
static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag);
static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group);
static int driver_write(neu_plugin_t *plugin, void *req, neu_datatag_t *tag, neu_value_u value);
static int driver_write_tags(neu_plugin_t *plugin, void *req, UT_array *tags);
static int driver_del_tags(neu_plugin_t *plugin, int n_tag);
static const neu_plugin_intf_funs_t plugin_intf_funs = {
        .open    = driver_open,
        .close   = driver_close,
        .init    = driver_init,
        .uninit  = driver_uninit,
        .start   = driver_start,
        .stop    = driver_stop,
        .setting = driver_config,
        .request = driver_request,

        .driver.validate_tag  = driver_validate_tag,
        .driver.group_timer   = driver_group_timer,
        .driver.write_tag     = driver_write,
        .driver.tag_validator = driver_tag_validator,
        .driver.write_tags    = driver_write_tags,
        .driver.add_tags      = NULL,
        .driver.load_tags     = NULL,
        .driver.del_tags      = driver_del_tags,
};

const neu_plugin_module_t neu_plugin_module = {
        .version         = NEURON_PLUGIN_VER_1_0,
        .schema          = "OPC-UA",
        .module_name     = "OPC-UA",
        .module_descr    = "This plugin is used to connect devices using the OPC UA protocol",
        .module_descr_zh = "该插件用于连接使用 OPC UA 协议的设备。",
        .intf_funs       = &plugin_intf_funs,
        .kind            = NEU_PLUGIN_KIND_SYSTEM,
        .type            = NEU_NA_TYPE_DRIVER,
        .display         = true,
        .single          = false,
};

static neu_plugin_t *driver_open(void)
{
    nlog_debug("\n==============================driver_open"
               "===========================\n");
    neu_plugin_t *plugin = calloc(1, sizeof(neu_plugin_t));

    neu_plugin_common_init(&plugin->common);

    return plugin;
}

static int driver_close(neu_plugin_t *plugin)
{
    plog_debug(plugin,
               "\n==============================driver_close"
               "===========================\n");
    free(plugin);

    return 0;
}

static int driver_init(neu_plugin_t *plugin, bool load)
{
    plog_debug(plugin,
               "\n==============================driver_init"
               "===========================\n");
    (void) load;
    plugin->ua_tag_hash_table_head = NULL;
    plog_notice(plugin, "%s init success", plugin->common.name);
    return 0;
}

static int driver_uninit(neu_plugin_t *plugin)
{
    plog_debug(plugin,
               "\n==============================driver_uninit"
               "===========================\n");
    plog_notice(plugin, "%s uninit start", plugin->common.name);

    plugin_free_all_tags(plugin);
    ua_client_stop(plugin);
    plog_notice(plugin, "%s uninit success", plugin->common.name);
    return 0;
}

static int driver_start(neu_plugin_t *plugin)
{
    plog_debug(plugin,
               "\n==============================driver_start"
               "===========================\n");

    ua_client_start(plugin);
    plugin->started = true;
    return 0;
}

static int driver_stop(neu_plugin_t *plugin)
{
    plog_debug(plugin,
               "\n==============================driver_stop"
               "===========================\n");
    plog_notice(plugin, "%s stop success", plugin->common.name);
    plugin->started = false;
    ua_client_stop(plugin);
    return 0;
}

static int driver_config(neu_plugin_t *plugin, const char *config)
{
    plog_debug(plugin,
               "\n==============================driver_config"
               "===========================\n");
    int   ret       = 0;
    char *err_param = NULL;

    //    neu_json_elem_t slot = { .name = "slot", .t = NEU_JSON_INT };

    neu_json_elem_t timeout        = { .name = "timeout", .t = NEU_JSON_INT };
    neu_json_elem_t host           = { .name = "host", .t = NEU_JSON_STR, .v.val_str = NULL };

    ret = neu_parse_param((char *) config, &err_param, 2,  &host, &timeout);

    if (ret != 0) {
        plog_error(plugin, "config: %s, decode error: %s", config, err_param);
        free(err_param);
        if (host.v.val_str != NULL) {
            free(host.v.val_str);
        }
        return -1;
    }

    if (timeout.v.val_int <= 0) {
        plog_error(plugin, "config: %s, set timeout error: %s", config, err_param);
        free(err_param);
        return -1;
    }

    plugin->host=host.v.val_str;
    return 0;
}

static int driver_request(neu_plugin_t *plugin, neu_reqresp_head_t *head, void *data)
{
    plog_debug(plugin,
               "\n==============================driver_request"
               "===========================\n");
    (void) plugin;
    (void) head;
    (void) data;
    return 0;
}
static int driver_tag_validator(const neu_datatag_t *tag)
{
    return 0;
}
static int driver_validate_tag(neu_plugin_t *plugin, neu_datatag_t *tag)
{
    plog_debug(plugin,
               "\n==============================driver_validate_tag"
               "===========================\n");
    // 打印tag的信息
    plog_debug(plugin,
               "tag: %s, address: %s, attribute: %d, type: %d, "
               "precision: %d, decimal: %f, description: %s",
               tag->name, tag->address, tag->attribute, tag->type, tag->precision, tag->decimal, tag->description);
    // 支持的类型校验
    if((tag->type>=1 && tag->type<=10)||tag->type==NEU_TYPE_BOOL){
        return 0;
    }
    return NEU_ERR_TAG_TYPE_NOT_SUPPORT;
}

static int driver_group_timer(neu_plugin_t *plugin, neu_plugin_group_t *group)
{
    plog_debug(plugin,
               "\n==============================driver_group_timer"
               "===========================\n");
    if(plugin->started == false){
        return NEU_ERR_NODE_NOT_RUNNING;
    }
    utarray_foreach(group->tags, neu_datatag_t *, tag)
    {
        handle_tag(plugin, tag, group);
    }
    return 0;
}
static int driver_write(neu_plugin_t *plugin, void *req, neu_datatag_t *tag, neu_value_u value)
{
    plog_debug(plugin,
               "\n==============================driver_write"
               "===========================\n");
    plog_debug(plugin, "write tag: %s, address: %s", tag->name, tag->address);
    return 0;
}

static int driver_write_tags(neu_plugin_t *plugin, void *req, UT_array *tags)
{
    plog_debug(plugin,
               "\n==============================driver_write_tags"
               "===========================\n");
    return 0;
}
static int driver_del_tags(neu_plugin_t *plugin, int n_tag)
{
    plog_debug(plugin,
               "\n==============================driver_del_tags"
               "===========================\n");
    plog_debug(plugin, "deleting %d tags", n_tag);
    return 0;
}