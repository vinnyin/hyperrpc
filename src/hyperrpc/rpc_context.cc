/* Copyright (c) 2016, Bin Wei <bin@vip.qq.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * The name of of its contributors may not be used to endorse or 
 * promote products derived from this software without specific prior 
 * written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "hyperrpc/rpc_context.h"

namespace hrpc {

static google::protobuf::ArenaOptions BuildArenaOptions(char* init_block,
                                                        size_t block_size)
{
  google::protobuf::ArenaOptions options;
  options.initial_block = init_block;
  options.initial_block_size = block_size;
  return options;
}

IncomingRpcContext::IncomingRpcContext(
                        const google::protobuf::MethodDescriptor* method,
                        const google::protobuf::Message& req_prot,
                        const google::protobuf::Message& resp_prot,
                        uint64_t rpc_id,
                        const Addr& addr)
  : arena_(BuildArenaOptions(arena_buf_, sizeof(arena_buf_)))
  , method_(method)
  , request_(req_prot.New(&arena_))
  , response_(resp_prot.New(&arena_))
  , rpc_id_(rpc_id)
  , addr_(addr)
{
}

IncomingRpcContext::~IncomingRpcContext()
{
}

} // namespace hrpc
