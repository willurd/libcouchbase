/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 *     Copyright 2010 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include "internal.h"

/**
 * Send a delete command to the correct server
 *
 * @author Trond Norbye
 * @todo improve the error handling
 */
LIBCOUCHBASE_API
libcouchbase_error_t libcouchbase_remove(libcouchbase_t instance,
                                         const void *key, size_t nkey,
                                         uint64_t cas)
{
    return libcouchbase_remove_by_key(instance, NULL, 0, key, nkey, cas);
}

LIBCOUCHBASE_API
libcouchbase_error_t libcouchbase_remove_by_key(libcouchbase_t instance,
                                                const void *hashkey,
                                                size_t nhashkey,
                                                const void *key, size_t nkey,
                                                uint64_t cas)
{
    uint16_t vb;
    libcouchbase_server_t *server;
    protocol_binary_request_delete req;

    // we need a vbucket config before we can start removing the item..
    libcouchbase_ensure_vbucket_config(instance);
    assert(instance->vbucket_config);

    if (nhashkey != 0) {
        vb = (uint16_t)vbucket_get_vbucket_by_key(instance->vbucket_config,
                                                  hashkey, nhashkey);
    } else {
        vb = (uint16_t)vbucket_get_vbucket_by_key(instance->vbucket_config,
                                                  key, nkey);
    }

    server = instance->servers + instance->vb_server_map[vb];
    memset(&req, 0, sizeof(req));
    req.message.header.request.magic = PROTOCOL_BINARY_REQ;
    req.message.header.request.opcode = PROTOCOL_BINARY_CMD_DELETE;
    req.message.header.request.keylen = ntohs((uint16_t)nkey);
    req.message.header.request.extlen = 0;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket = ntohs(vb);
    req.message.header.request.bodylen = ntohl((uint32_t)nkey);
    req.message.header.request.opaque = ++instance->seqno;
    req.message.header.request.cas = cas;

    libcouchbase_server_start_packet(server, req.bytes, sizeof(req.bytes));
    libcouchbase_server_write_packet(server, key, nkey);
    libcouchbase_server_end_packet(server);
    libcouchbase_server_send_packets(server);

    return LIBCOUCHBASE_SUCCESS;
}
