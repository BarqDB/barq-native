////////////////////////////////////////////////////////////////////////////
//
// Copyright 2024 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#ifndef BARQ_NATIVE_NETWORKING_UTILS_HPP
#define BARQ_NATIVE_NETWORKING_UTILS_HPP

#include <barq_native/networking/websocket.hpp>

#include <optional>

namespace barq {
    namespace app {
        struct Request;
        struct Response;
    }
    namespace sync {
        struct WebSocketEndpoint;
    }
}

namespace barq::native::networking {
    struct request;
    struct response;
}

namespace barq::native::internal::networking {
    ::barq::native::networking::request to_request(const ::barq::app::Request&);
    ::barq::app::Response to_core_response(const ::barq::native::networking::response&);

    ::barq::sync::WebSocketEndpoint to_core_websocket_endpoint(const ::barq::native::networking::websocket_endpoint& ep,
                                                                const std::optional<::barq::native::networking::default_socket_provider::configuration>& config);
    ::barq::native::networking::websocket_endpoint to_websocket_endpoint(const ::barq::sync::WebSocketEndpoint& ep);
} //namespace barq::native::internal::networking

#endif //BARQ_NATIVE_NETWORKING_UTILS_HPP
