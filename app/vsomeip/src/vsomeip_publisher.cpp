#include <vsomeip/vsomeip.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

// IDs
constexpr vsomeip::service_t  SERVICE_ID  = 0x1234;
constexpr vsomeip::instance_t INSTANCE_ID = 0x5678;
constexpr vsomeip::event_t    EVENT_ID    = 0x0421;
constexpr vsomeip::eventgroup_t EVENTGROUP_ID = 0x0001;

int main()
{
    // 1. Create application
    auto app = vsomeip::runtime::get()->create_application("vsomeip_publisher");

    // 2. Initialize
    if (!app->init()) {
        std::cerr << "Failed to initialize application\n";
        return 1;
    }

    // 3. Register state handler
    app->register_state_handler(
        [&](vsomeip::state_type_e state)
        {
            if (state == vsomeip::state_type_e::ST_REGISTERED)
            {
                std::cout << "Application registered\n";

                // 4. Offer service
                app->offer_service(SERVICE_ID, INSTANCE_ID);

                // 5. Offer event
                std::set<vsomeip::eventgroup_t> groups{EVENTGROUP_ID};

                /*app->register_event(
                    SERVICE_ID,
                    INSTANCE_ID,
                    EVENT_ID,
                    groups,
                    vsomeip::event_type_e::ET_FIELD
                );*/
                
                app->offer_event(
                    SERVICE_ID,
                    INSTANCE_ID,
                    EVENT_ID,
                    groups,
                    vsomeip::event_type_e::ET_FIELD
                );

                std::cout << "Service and event offered\n";
            }
        }
    );

    // 6. Start vSomeIP (runs internal threads)
    std::thread app_thread([&]() {
        app->start();
    });

    // 7. Publish loop
    uint32_t counter = 0;

    while (true)
    {
        // Create payload
        std::string text = "Hello SOME/IP " + std::to_string(counter++);

        auto payload = vsomeip::runtime::get()->create_payload();
        payload->set_data(
            reinterpret_cast<const uint8_t*>(text.data()),
            static_cast<uint32_t>(text.size())
        );

        // 8. Notify subscribers
        app->notify(
            SERVICE_ID,
            INSTANCE_ID,
            EVENT_ID,
            payload
        );

        std::cout << "Sent: " << text << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    app_thread.join();
    return 0;
}