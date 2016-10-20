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
#include <algorithm>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "hyperrpc/rpc_session_manager.h"

namespace hrpc {

RpcSessionManager::RpcSessionManager(const Env& env)
  : env_(env)
{
}

RpcSessionManager::~RpcSessionManager()
{
}

bool RpcSessionManager::Init(size_t sess_pool_size,
                             size_t rpc_core_id,
                             OnSendRequest on_send_req)
{
  // align pool size to 2^pool_size_order_
  size_t pool_size_order = 0;
  while ((1UL << pool_size_order) < sess_pool_size) {
    pool_size_order++;
  }
  pool_size_order_ = std::min(pool_size_order, kRpcIdSeqPartBits);
  pool_size_mask_ = (1UL << pool_size_order) - 1;
  // allocate session pool
  sess_pool_.reset(new SessionNode[1UL << pool_size_order_]);
  // initialize RPC_ID = CORE_ID_PLUS_1(16bit) + RANDOM(32bit) + ZEROS(16bit)
  next_rpc_id_ = ((uint64_t)(rpc_core_id + 1) << kRpcIdSeqPartBits) +
                 ((uint64_t)env_.Rand() << (kRpcIdSeqPartBits - 32));
  // set callback
  on_send_request_ = on_send_req;
  return true;
}

bool RpcSessionManager::AddSession(
                          const ::google::protobuf::MethodDescriptor* method,
                          const ::google::protobuf::Message* request,
                          ::google::protobuf::Message* response,
                          const EndpointList& endpoint_list,
                          ::ccb::ClosureFunc<void(Result)> done)
{
  SessionNode* node = AllocSessionNode();
  if (!node) {
    WLOG("AllocSessionNode failed!");
    done(kInError);
    return false;
  }
  node->method = method;
  node->request = request;
  node->response = response;
  node->done = std::move(done);
  node->endpoint_list = endpoint_list;
  // todo: set timer
  on_send_request_(method, *request, node->rpc_id,
                   endpoint_list.GetEndpoint(0));
  return true;
}

void RpcSessionManager::OnRecvResponse(const std::string& service,
                                       const std::string& method,
                                       uint64_t rpc_id,
                                       Result rpc_result,
                                       const Buf& resp_body)
{
  SessionNode* node = FindSessionNode(rpc_id);
  if (!node) {
    ILOG("OnRecvResponse: cannot find session node");
    return;
  }
  if (node->method->service()->name() != service ||
      node->method->name() != method) {
    ILOG("OnRecvResponse: node found but service-method name dismatch");
    return;
  }
  if (!node->response->ParseFromArray(resp_body.ptr(), resp_body.len())) {
    ILOG("OnRecvResponse: parse response message failed");
    return;
  }
  // rpc done callback
  node->done(rpc_result);
  // cleanup
  node->timer_owner.Cancel();
  FreeSessionNode(node);
}

RpcSessionManager::SessionNode* RpcSessionManager::AllocSessionNode()
{
  for (size_t i = 0; i < (1UL << pool_size_order_); i++) {
    uint64_t rpc_id = NextRpcId();
    SessionNode* node = &sess_pool_[rpc_id & pool_size_mask_];
    if (node->rpc_id == 0) {
      // got empty node
      node->rpc_id = rpc_id;
      return node;
    }
  }
  return nullptr;
}

RpcSessionManager::SessionNode* RpcSessionManager::FindSessionNode(
                                                   uint64_t rpc_id)
{
  SessionNode* node = &sess_pool_[rpc_id & pool_size_mask_];
  if (node->rpc_id == rpc_id) {
    return node;
  } else {
    ILOG("rpc_id dismatch when finding session node");
    return nullptr;
  }
}

void RpcSessionManager::FreeSessionNode(SessionNode* node)
{
  node->rpc_id = 0;
}

} // namespace hrpc
