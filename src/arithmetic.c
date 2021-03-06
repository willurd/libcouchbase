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
 * Spool an arithmetic request
 *
 * @author Trond Norbye
 * @todo add documentation
 */
LIBCOUCHBASE_API
libcouchbase_error_t libcouchbase_arithmetic(libcouchbase_t instance,
                                             const void *key, size_t nkey,
                                             int64_t delta, time_t exp,
                                             bool create, uint64_t initial)
{
    return libcouchbase_arithmetic_by_key(instance, NULL, 0, key, nkey,
                                          delta, exp, create, initial);
}

LIBCOUCHBASE_API
libcouchbase_error_t libcouchbase_arithmetic_by_key(libcouchbase_t instance,
                                                    const void *hashkey,
                                                    size_t nhashkey,
                                                    const void *key, size_t nkey,
                                                    int64_t delta, time_t exp,
                                                    bool create, uint64_t initial)
{
    uint16_t vb;
    libcouchbase_server_t *server;
    protocol_binary_request_incr req;

    // we need a vbucket config before we can start getting data..
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
    req.message.header.request.opcode = PROTOCOL_BINARY_CMD_INCREMENT;
    req.message.header.request.keylen = ntohs((uint16_t)nkey);
    req.message.header.request.extlen = 20;
    req.message.header.request.datatype = PROTOCOL_BINARY_RAW_BYTES;
    req.message.header.request.vbucket = ntohs(vb);
    req.message.header.request.bodylen = ntohl((uint32_t)(nkey + 20));
    req.message.header.request.opaque = ++instance->seqno;
    req.message.body.delta = ntohll((uint64_t)(delta));
    req.message.body.initial = ntohll(initial);
    req.message.body.expiration = ntohl((uint32_t)exp);

    if (delta < 0) {
        req.message.header.request.opcode = PROTOCOL_BINARY_CMD_DECREMENT;
        req.message.body.delta = ntohll((uint64_t)(delta * -1));
    }

    if (create) {
        memset(&req.message.body.expiration, 0xff,
               sizeof(req.message.body.expiration));
    }

    libcouchbase_server_start_packet(server, req.bytes, sizeof(req.bytes));
    libcouchbase_server_write_packet(server, key, nkey);
    libcouchbase_server_end_packet(server);
    libcouchbase_server_send_packets(server);

    return LIBCOUCHBASE_SUCCESS;
}
