#include <barq_native/internal/networking/shims.hpp>
#include <barq_native/internal/networking/utils.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/sync/network/default_socket.hpp>
#include <barq/util/platform_info.hpp>

namespace barq::native::internal::networking {

    std::unique_ptr<::barq::sync::WebSocketInterface> create_websocket_interface_shim(std::unique_ptr<::barq::native::networking::websocket_interface>&& m_interface) {
        struct core_websocket_interface_shim : public ::barq::sync::WebSocketInterface {
            ~core_websocket_interface_shim() = default;
            explicit core_websocket_interface_shim(std::unique_ptr<::barq::native::networking::websocket_interface>&& ws) : m_interface(std::move(ws)) {}

            void async_write_binary(util::Span<const char> data, sync::SyncSocketProvider::FunctionHandler&& handler) override {
                auto handler_ptr = handler.release();
                auto b = std::string_view(data.data(), data.size());
                m_interface->async_write_binary(b, [ptr = std::move(handler_ptr)](::barq::native::networking::status s) {
                    auto uf = util::UniqueFunction<void(::barq::Status)>(std::move(ptr));
                    return uf(s.operator ::barq::Status());
                });
            };

            std::unique_ptr<::barq::native::networking::websocket_interface> m_interface;
        };

        return std::make_unique<core_websocket_interface_shim>(std::move(m_interface));
    }

    std::shared_ptr<app::GenericNetworkTransport> create_http_client_shim(const std::shared_ptr<::barq::native::networking::http_transport_client>& http_client) {
        struct core_http_transport_shim : app::GenericNetworkTransport {
            ~core_http_transport_shim() = default;
            core_http_transport_shim(const std::shared_ptr<::barq::native::networking::http_transport_client>& http_client) {
                m_http_client = http_client;
            }

            void send_request_to_server(const app::Request& request,
                                        util::UniqueFunction<void(const app::Response&)>&& completion) {
                auto completion_ptr = completion.release();
                m_http_client->send_request_to_server(to_request(request),
                                                      [f = std::move(completion_ptr)]
                                                              (const ::barq::native::networking::response& response) {
                                                          auto uf = util::UniqueFunction<void(const app::Response&)>(std::move(f));
                                                          uf(to_core_response(response));
                                                      });
            }

        private:
            std::shared_ptr<::barq::native::networking::http_transport_client> m_http_client;
        };

        return std::make_shared<core_http_transport_shim>(http_client);
    }

    std::unique_ptr<::barq::native::networking::websocket_observer> create_websocket_observer_from_core_shim(std::unique_ptr<::barq::sync::WebSocketObserver>&& m_observer) {
        struct core_websocket_observer_shim : public ::barq::native::networking::websocket_observer {
            explicit core_websocket_observer_shim(std::unique_ptr<::barq::sync::WebSocketObserver>&& ws) : m_observer(std::move(ws)) {}
            ~core_websocket_observer_shim() = default;

            void websocket_connected_handler(const std::string& protocol) override {
                m_observer->websocket_connected_handler(protocol);
            }

            void websocket_error_handler() override {
                m_observer->websocket_error_handler();
            }

            bool websocket_binary_message_received(std::string_view data) override {
                return m_observer->websocket_binary_message_received(data);
            }

            bool websocket_closed_handler(bool was_clean, ::barq::native::networking::websocket_err_codes error_code,
                                          std::string_view message) override {
                return m_observer->websocket_closed_handler(was_clean, static_cast<::barq::sync::websocket::WebSocketError>(error_code), message);
            }

            std::unique_ptr<::barq::sync::WebSocketObserver> m_observer;
        };

        return std::make_unique<core_websocket_observer_shim>(std::move(m_observer));
    }

    std::unique_ptr<::barq::sync::SyncSocketProvider> create_sync_socket_provider_shim(const std::shared_ptr<::barq::native::networking::sync_socket_provider>& provider) {
        struct sync_timer_shim final : public ::barq::sync::SyncSocketProvider::Timer {
            sync_timer_shim(std::unique_ptr<::barq::native::networking::sync_socket_provider::timer>&& timer) : m_timer(std::move(timer)) {}
            ~sync_timer_shim() = default;

            void cancel() override {
                m_timer->cancel();
            }

        private:
            std::unique_ptr<::barq::native::networking::sync_socket_provider::timer> m_timer;
        };

        struct sync_socket_provider_shim final : public ::barq::sync::SyncSocketProvider {
            explicit sync_socket_provider_shim(const std::shared_ptr<::barq::native::networking::sync_socket_provider>& provider) {
                m_provider = provider;
            }

            sync_socket_provider_shim() = delete;
            ~sync_socket_provider_shim() = default;

            std::unique_ptr<::barq::sync::WebSocketInterface> connect(std::unique_ptr<::barq::sync::WebSocketObserver> observer, ::barq::sync::WebSocketEndpoint&& ep) override {
                auto barq_native_ws_interface = m_provider->connect(create_websocket_observer_from_core_shim(std::move(observer)),
                                                                 to_websocket_endpoint(std::move(ep)));
                return create_websocket_interface_shim(std::move(barq_native_ws_interface));
            }

            void post(FunctionHandler&& handler) override {
                auto handler_ptr = handler.release();
                m_provider->post([ptr = std::move(handler_ptr)](::barq::native::networking::status s) {
                    auto uf = util::UniqueFunction<void(::barq::Status)>(std::move(ptr));
                    return uf(s.operator ::barq::Status());
                });
            }

            ::barq::sync::SyncSocketProvider::SyncTimer create_timer(std::chrono::milliseconds delay, ::barq::sync::SyncSocketProvider::FunctionHandler&& handler) override {
                auto handler_ptr = handler.release();
                auto fn = [ptr = std::move(handler_ptr)](::barq::native::networking::status s) {
                    auto uf = util::UniqueFunction<void(::barq::Status)>(std::move(ptr));
                    return uf(s.operator ::barq::Status());
                };
                return  std::make_unique<sync_timer_shim>(m_provider->create_timer(delay, std::move(fn)));
            }
        private:
            std::shared_ptr<::barq::native::networking::sync_socket_provider> m_provider;
        };

        return std::make_unique<sync_socket_provider_shim>(std::move(provider));
    }
} //namespace internal::networking