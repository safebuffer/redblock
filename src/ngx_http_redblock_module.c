
/*
 * Copyright (C) safebuffer 2022
 */

#include <ngx_http_redblock_module.h>

/**
 * Merges the configuration settings of the parent and child configurations
 * for the ngx_http_redblock module.
 *
 * This function is used during the configuration merge phase to ensure that
 * the "blocked" configuration option is properly merged between parent and child
 * configurations.
 *
 * @param cf      The ngx_conf_t configuration structure.
 * @param parent  The parent configuration structure.
 * @param child   The child configuration structure.
 *
 * @return NGX_CONF_OK on successful merging.
 */
static char *ngx_http_redblock_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    // Casting parent and child to ngx_http_redblock_conf_t pointers
    ngx_http_redblock_conf_t  *prev = (ngx_http_redblock_conf_t *)parent;
    ngx_http_redblock_conf_t  *conf = (ngx_http_redblock_conf_t *)child;

    // Merging the "blocked" configuration option from the parent and child configurations
    ngx_conf_merge_ptr_value(conf->blocked, prev->blocked, NULL);
    ngx_conf_merge_ptr_value(conf->blocked, prev->blocked, NULL);

    // Returning NGX_CONF_OK to indicate successful merging
    return NGX_CONF_OK;
}

/**
 * Initializes the ngx_http_redblock module.
 *
 * This function is responsible for registering the ngx_http_redblock_handler
 * to be invoked during the NGX_HTTP_PREACCESS_PHASE phase.
 *
 * @param cf The ngx_conf_t configuration structure.
 *
 * @return NGX_OK if initialization is successful, NGX_ERROR otherwise.
 */
static ngx_int_t ngx_http_redblock_module_init(ngx_conf_t *cf) {
    // Retrieve the core module's main configuration
    ngx_http_core_main_conf_t   *cmcf;
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    // Add the ngx_http_redblock_handler to the NGX_HTTP_PREACCESS_PHASE handlers
    ngx_http_handler_pt         *h;
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_PREACCESS_PHASE].handlers);
    
    // Check if the handler addition is successful
    if (h == NULL) {
        return NGX_ERROR;
    }

    // Set the ngx_http_redblock_handler as the handler for NGX_HTTP_PREACCESS_PHASE
    *h = ngx_http_redblock_handler;

    // Return NGX_OK to indicate successful initialization
    return NGX_OK;
}


static char *ngx_http_redblock(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    return NGX_CONF_OK;
}


/**
 * Allocates and initializes the configuration structure for the ngx_http_redblock module.
 *
 * @param cf The ngx_conf_t configuration structure.
 * @return A pointer to the newly created ngx_http_redblock_conf_t structure.
 */
static void * ngx_http_redblock_loc_conf(ngx_conf_t *cf)
{
    ngx_http_redblock_conf_t  *conf;
    ip_range_t range;
    char* filename = "/etc/nginx/redblock_ranges.bin";

    // Allocate memory for the module's configuration structure
    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_redblock_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    // Initialize the ngx_array for blocked IP ranges
    if (conf->blocked == NULL) {
        conf->blocked = ngx_array_create(cf->pool, 10, sizeof(ip_range_t));
        if (conf->blocked == NULL) {
            ngx_log_error(NGX_LOG_CRIT, cf->log, 0,
                          "[REDBLOCK] Failed to allocate memory for the dataset.");
            return NGX_CONF_ERROR;
        }
    }
    ngx_array_init(conf->blocked, cf->pool, 10, sizeof(ip_range_t));

    // Open the binary file containing IP ranges
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        return NGX_CONF_ERROR;
    }

    // Read IP ranges from the file and add them to the ngx_array
    while (fread(&range, sizeof(ip_range_t), 1, file) == 1) {
        range.start = (range.start);
        range.end = (range.end);
        range.type = (range.type);

        ip_range_t *newRange = ngx_array_push(conf->blocked);
        if (newRange == NULL) {
            break;
        }
        *newRange = range;
    }

    fclose(file);
    return conf;
}

/**
 * Checks if the given IPv4 address is within any blocked IP range and denies access accordingly.
 *
 * This function is used to determine if a request with the provided IPv4 address
 * should be blocked based on the configured blocked IP ranges.
 *
 * @param r     The ngx_http_request_t request structure.
 * @param user_adr  The IPv4 address to be checked.
 *
 * @return NGX_OK if the address is not blocked, NGX_HTTP_UNAUTHORIZED if blocked,
 *         or an appropriate error code if there's an issue with the configuration.
 */
static ngx_int_t ngx_http_access_ipv4(ngx_http_request_t *r, unsigned int user_adr)
{
    // Retrieve the ngx_http_redblock_conf_t configuration for the current location
    ngx_http_redblock_conf_t  *rel;
    rel = ngx_http_get_module_loc_conf(r, ngx_http_redblock_module);

    // Check if the configuration is successfully loaded
    if (rel == NULL) {
        ngx_log_error(NGX_LOG_CRIT, r->connection->log, 0,
                      "[REDBLOCK] Can't load the IP ranges dataset, will ignore the requests.");
        return NGX_OK;
    }

    // Iterate through the blocked IP ranges and check if the given address is within any range
    ngx_uint_t j;
    ip_range_t *blocked_range;
    blocked_range = rel->blocked->elts;

    for (j = 0; j < rel->blocked->nelts; j++) {
        if (user_adr >= blocked_range[j].start && user_adr <= blocked_range[j].end) {
            // The address is within a blocked range, log and deny access
            ngx_log_error(NGX_LOG_ALERT, r->connection->log, 0,
                          "[REDBLOCK] Request Blocked");
            return NGX_HTTP_UNAUTHORIZED;
        }
    }
    // The address is not within any blocked range, allow access
    return NGX_OK;
}

/**
 * NGX_HTTP_REDBLOCK module handler for processing IP addresses.
 *
 * This handler determines the address family of the incoming request's connection
 * and calls the appropriate function for handling IPv4 or IPv6 addresses.
 *
 * @param r The ngx_http_request_t request structure.
 * @return NGX_OK if the processing is successful.
 */
static ngx_int_t ngx_http_redblock_handler(ngx_http_request_t *r) {
    struct sockaddr_in  *sin;

#if (NGX_HAVE_INET6)
    u_char               *p;
    in_addr_t             addr;
    struct sockaddr_in6  *sin6;
#endif

    switch (r->connection->sockaddr->sa_family) {

    case AF_INET:
        // Handling IPv4 address
        sin = (struct sockaddr_in *) r->connection->sockaddr;
        return ngx_http_access_ipv4(r, htonl(sin->sin_addr.s_addr));
        break;

#if (NGX_HAVE_INET6)
    case AF_INET6:
        // Handling IPv6 address
        sin6 = (struct sockaddr_in6 *) r->connection->sockaddr;
        p = sin6->sin6_addr.s6_addr;

        if (IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr)) {
            // Convert IPv4-mapped IPv6 address to IPv4
            addr = p[12] << 24;
            addr += p[13] << 16;
            addr += p[14] << 8;
            addr += p[15];
            return ngx_http_access_ipv4(r, htonl(addr));
        }

        // TODO: Add native IPv6 support
        return NGX_OK;
        break;

#endif
    }

    return NGX_OK;
}



