/*
 * Copyright (C) safebuffer 2022
 */

#ifndef _NGX_http_redblock_MODULE_H_
#define _NGX_http_redblock_MODULE_H_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_log.h>

#pragma pack(1)  // Disable padding

/**
 * Structure representing an IP range with start, end, and type information.
 */
typedef struct {
    unsigned int start; /**< Start of the IP range */
    unsigned int end;   /**< End of the IP range */
    unsigned int type;  /**< Type of the IP range */
} ip_range_t;

/**
 * Configuration structure for the ngx_http_redblock module.
 */
typedef struct {
    ngx_array_t  *blocked; /**< Array of ip_range_t representing blocked IP ranges */
} ngx_http_redblock_conf_t;

#pragma pack()  // Restore default packing

/**
 * Handler function for processing HTTP requests in the ngx_http_redblock module.
 *
 * @param r The ngx_http_request_t request structure.
 * @return NGX_OK if the processing is successful.
 */
static ngx_int_t ngx_http_redblock_handler(ngx_http_request_t *r);

/**
 * Command handler function for the "redblock" directive in the configuration.
 *
 * @param cf The ngx_conf_t configuration structure.
 * @param cmd The ngx_command_t command structure.
 * @param conf The configuration structure.
 * @return A string pointer indicating the result of the command.
 */
static char *ngx_http_redblock(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

/**
 * Initialization function for the ngx_http_redblock module.
 *
 * @param cf The ngx_conf_t configuration structure.
 * @return NGX_OK if the initialization is successful.
 */
static ngx_int_t ngx_http_redblock_module_init(ngx_conf_t *cf);

/**
 * Creates the location-specific configuration structure for the ngx_http_redblock module.
 *
 * @param cf The ngx_conf_t configuration structure.
 * @return A pointer to the created ngx_http_redblock_conf_t structure.
 */
static void *ngx_http_redblock_loc_conf(ngx_conf_t *cf);

/**
 * Merges the location-specific configuration structures for the ngx_http_redblock module.
 *
 * @param cf The ngx_conf_t configuration structure.
 * @param parent The parent configuration structure.
 * @param child The child configuration structure.
 * @return NGX_CONF_OK if the merging is successful.
 */
static char *ngx_http_redblock_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

/**
 * Array of ngx_command_t structures defining module directives.
 */
static ngx_command_t ngx_http_redblock_commands[] = {
    {
        ngx_string("redblock"),              /* name */
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS,    /* type */
        ngx_http_redblock,                   /* set */
        0,                                      /* conf */
        0,                                      /* offset */
        NULL                                    /* post */
    },
    ngx_null_command
};

/**
 * Module context structure for the ngx_http_redblock module.
 */
static ngx_http_module_t ngx_http_redblock_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_http_redblock_module_init,   /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_http_redblock_loc_conf,                               /* create location configuration */
    ngx_http_redblock_merge_loc_conf                                /* merge location configuration */
};

/**
 * Main ngx_module_t structure for the ngx_http_redblock module.
 */
ngx_module_t ngx_http_redblock_module = {
    NGX_MODULE_V1,
    &ngx_http_redblock_module_ctx, /* module context */
    ngx_http_redblock_commands,    /* module directives */
    NGX_HTTP_MODULE,                  /* module type */
    NULL,                             /* init master */
    NULL,                             /* init module */
    NULL,                             /* init process */
    NULL,                             /* init thread */
    NULL,                             /* exit thread */
    NULL,                             /* exit process */
    NULL,                             /* exit master */
    NGX_MODULE_V1_PADDING
};

#endif
