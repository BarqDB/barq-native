#include <barq_native/internal/networking/utils.hpp>
#include <barq_native/networking/http.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/sync/protocol.hpp>
#include <barq/sync/socket_provider.hpp>
#include <barq/util/base64.hpp>
#include <barq/util/uri.hpp>

namespace barq::native::internal::networking {

    ::barq::native::networking::request to_request(const ::barq::app::Request& core_request) {
        ::barq::native::networking::request req;
        req.method = static_cast<::barq::native::networking::http_method>(core_request.method);
        req.url = core_request.url;
        req.timeout_ms = core_request.timeout_ms;
        req.headers = core_request.headers;
        req.body = core_request.body;
        return req;
    }

    ::barq::app::Response to_core_response(const ::barq::native::networking::response& req) {
        ::barq::app::Response core_response;
        core_response.http_status_code = req.http_status_code;
        core_response.custom_status_code = req.custom_status_code;
        core_response.headers = req.headers;
        core_response.body = req.body;
        if (req.client_error_code) {
            core_response.client_error_code = static_cast<::barq::ErrorCodes::Error>(*req.client_error_code);
        }
        return core_response;
    }

    ::barq::sync::ProtocolEnvelope to_protocol_envelope(const std::string& protocol) noexcept
    {
        if (protocol == "barq:") {
            return ::barq::sync::ProtocolEnvelope::barq;
        } else if (protocol == "barqs:") {
            return ::barq::sync::ProtocolEnvelope::barqs;
        } else if (protocol == "ws:") {
            return ::barq::sync::ProtocolEnvelope::ws;
        } else if (protocol == "wss:") {
            return ::barq::sync::ProtocolEnvelope::wss;
        }
        BARQ_TERMINATE("Unrecognized websocket protocol");
    }

    ::barq::sync::WebSocketEndpoint to_core_websocket_endpoint(const ::barq::native::networking::websocket_endpoint& ep,
                                                                const std::optional<::barq::native::networking::default_socket_provider::configuration>& config) {
        ::barq::sync::WebSocketEndpoint core_ep;
        auto uri = util::Uri(ep.url);
        auto protocol = to_protocol_envelope(uri.get_scheme());

        std::string userinfo, host, port;
        uri.get_auth(userinfo, host, port);
        core_ep.is_ssl = ::barq::sync::is_ssl(protocol);

        core_ep.address = host;
        if (port.empty()) {
            core_ep.port = core_ep.is_ssl ? 443 : 80;
        } else {
            core_ep.port = std::stoi(port);
        }
        core_ep.path = uri.get_path() + uri.get_query();
        core_ep.protocols = ep.protocols;

        if (config) {
            if (config->proxy_config) {
                core_ep.proxy = SyncConfig::ProxyConfig();
                core_ep.proxy->address = config->proxy_config->address;
                core_ep.proxy->port = config->proxy_config->port;
                if (config->proxy_config->username_password) {
                    auto userpass = util::format("%1:%2", config->proxy_config->username_password->first, config->proxy_config->username_password->second);
                    std::string encoded_userpass;
                    encoded_userpass.resize(barq::util::base64_encoded_size(userpass.length()));
                    barq::util::base64_encode(userpass, encoded_userpass);
                    core_ep.headers.emplace("Proxy-Authorization", util::format("Basic %1", encoded_userpass));
                }
            }

            if (config->custom_http_headers) {
                core_ep.headers = *config->custom_http_headers;
            }

            core_ep.verify_servers_ssl_certificate = config->client_validate_ssl;
            core_ep.ssl_trust_certificate_path = config->ssl_trust_certificate_path;
            core_ep.ssl_verify_callback = config->ssl_verify_callback;
        }

        return core_ep;
    }

    ::barq::native::networking::websocket_endpoint to_websocket_endpoint(const ::barq::sync::WebSocketEndpoint& core_ep) {
        ::barq::native::networking::websocket_endpoint ep;
        ep.protocols = core_ep.protocols;
        const auto& port = core_ep.proxy ? core_ep.proxy->port : core_ep.port;
        ep.url = util::format("%1://%2:%3%4", core_ep.is_ssl ? "wss" : "ws", core_ep.address, port, core_ep.path);
        return ep;
    }
} //namespace internal::networking
