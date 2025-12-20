#include <vsomeip/vsomeip.hpp>

#include <iostream>

constexpr vsomeip::service_t     SERVICE_ID     = 0x1234;
constexpr vsomeip::instance_t    INSTANCE_ID    = 0x5678;
constexpr vsomeip::event_t       EVENT_ID       = 0x0421;
constexpr vsomeip::eventgroup_t  EVENTGROUP_ID  = 0x0001;

int main()
{
    auto app = vsomeip::runtime::get()->create_application("vsomeip_subscriber");

    if (!app->init()) {
        std::cerr << "Failed to init subscriber\n";
        return 1;
    }

    // Message handler
    app->register_message_handler(
        SERVICE_ID,
        INSTANCE_ID,
        EVENT_ID,
        [](const std::shared_ptr<vsomeip::message>& msg)
        {
            auto payload = msg->get_payload();
            std::string text(
                reinterpret_cast<const char*>(payload->get_data()),
                payload->get_length()
            );

            std::cout << "Received: " << text << std::endl;
        }
    );

    // State handler
    app->register_state_handler(
        [&](vsomeip::state_type_e state)
        {
            if (state == vsomeip::state_type_e::ST_REGISTERED)
            {
                std::cout << "Subscriber registered\n";

                // 1. Request service
                app->request_service(SERVICE_ID, INSTANCE_ID);

                std::set<vsomeip::eventgroup_t> groups{EVENTGROUP_ID};

                // 2. REQUEST EVENT (THIS IS CRITICAL)
                app->request_event(
                    SERVICE_ID,
                    INSTANCE_ID,
                    EVENT_ID,
                    groups,
                    vsomeip::event_type_e::ET_FIELD
                );

                // 3. Subscribe to event group
                app->subscribe(
                    SERVICE_ID,
                    INSTANCE_ID,
                    EVENTGROUP_ID
                );

                std::cout << "Event requested and subscribed\n";
            }
        }
    );

    app->start();
    return 0;
}
