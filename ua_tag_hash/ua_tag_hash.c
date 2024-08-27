//
// Created by root on 8/27/24.
//
#include "ua_tag_hash.h"
ua_tag_hash_t *plugin_find_tag(neu_plugin_t *plugin, char *key)
{
    ua_tag_hash_t *entry = NULL;
    HASH_FIND_STR(plugin->ua_tag_hash_table_head, key, entry);
    return entry;
}
void plugin_add_tag(neu_plugin_t *plugin, char *key, UA_NodeId node_id)
{
    ua_tag_hash_t *s = NULL;
    HASH_FIND_STR(plugin->ua_tag_hash_table_head, key, s);
    if (s == NULL) {
        s = (ua_tag_hash_t *) malloc(sizeof(ua_tag_hash_t));
        strcpy(s->name, key);
        s->value = node_id;
        HASH_ADD_STR(plugin->ua_tag_hash_table_head, name, s);
    }
}
void plugin_del_tag(neu_plugin_t *plugin, char *key)
{
    ua_tag_hash_t *s = plugin_find_tag(plugin, key);
    if (s != NULL) {
        HASH_DEL(plugin->ua_tag_hash_table_head, s);
        free(s);
    }
}
void plugin_free_all_tags(neu_plugin_t *plugin)
{
    if(plugin->ua_tag_hash_table_head == NULL){
        return;
    }
    ua_tag_hash_t *current_tag, *tmp;

    // HASH_ITER 宏可以安全地遍历哈希表，即使我们正在删除元素
    HASH_ITER(hh, plugin->ua_tag_hash_table_head, current_tag, tmp)
    {
        // 从哈希表中删除当前元素
        HASH_DEL(plugin->ua_tag_hash_table_head, current_tag);
        // 释放当前元素的内存
        free(current_tag);
    }
    plugin->ua_tag_hash_table_head = NULL;
}

ua_tag_hash_t *plugin_get_first_tag(neu_plugin_t *plugin)
{
    ua_tag_hash_t *s = NULL, *tmp;

    // 使用HASH_ITER宏遍历哈希表
    // 这里假设plugin->ua_tag_hash_table_head是哈希表头指针
    HASH_ITER(hh, plugin->ua_tag_hash_table_head, s, tmp)
    {
        // 一旦迭代到了第一个元素，立即返回该元素
        // 因为我们只想要第一个元素，不继续迭代
        return s;
    }

    // 如果哈希表为空，返回NULL
    return NULL;
}