#include <barq_native/networking/http.hpp>
#include <barq_native/internal/bridge/barq.hpp>

#include <barq/object-store/sync/generic_network_transport.hpp>
#include <barq/sync/network/default_socket.hpp>
#include <barq/util/platform_info.hpp>

namespace barq::native::networking {
    std::shared_ptr<http_transport_client> make_default_http_client() {
        return std::make_shared<networking::default_http_transport>();
    }
    std::function<std::shared_ptr<http_transport_client>()> s_http_client_factory = make_default_http_client;

    void set_http_client_factory(std::function<std::shared_ptr<http_transport_client>()>&& factory_fn) {
        s_http_client_factory = std::move(factory_fn);
    }

    std::shared_ptr<http_transport_client> make_http_client() {
        return s_http_client_factory();
    }
}
