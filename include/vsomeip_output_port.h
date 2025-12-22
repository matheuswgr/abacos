#ifndef __vsomeip_output_port_h
#define __vsomeip_output_port_h

#include <vsomeip/vsomeip.hpp>

#include <port_event.h>
#include <serializable.h>
#include <delegate.h>

#include <iostream>
#include <thread>
#include <chrono>

namespace abacos
{
    template <typename T>
    requires Serializable<T>
    class VSOMEIP_Output_Port
    {
    public:
        VSOMEIP_Output_Port(int port_identifier, std::string topic)
            : port_identifier_(port_identifier), topic_(topic)
        {
            application_ = vsomeip::runtime::get()->create_application("vsomeip_publisher");

            application_->init();

            application_->register_state_handler(
                [&](vsomeip::state_type_e state)
                {
                    if (state == vsomeip::state_type_e::ST_REGISTERED)
                    {
                        application_->offer_service(SERVICE_ID, INSTANCE_ID);

                        std::set<vsomeip::eventgroup_t> groups{EVENTGROUP_ID};

                        application_->offer_event(
                            SERVICE_ID,
                            INSTANCE_ID,
                            EVENT_ID,
                            groups,
                            vsomeip::event_type_e::ET_EVENT);
                    }
                });

                application_thread_ = std::thread([this]()
                                                  { application_->start(); });
        }

        void write(T data)
        {
            std::cout << "SEND " << data.serialize() << std::endl;
            
            std::shared_ptr<vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();

            data.pre_publisher_serialization_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            payload->set_data(reinterpret_cast<const uint8_t*>(data.serialize()),
            static_cast<uint32_t>(data.size_bytes()));

            data.post_publisher_serialization_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            data.vsomeip_publisher_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()
                ).count();

            application_->notify(SERVICE_ID, INSTANCE_ID, EVENT_ID, payload);
        }

    private:
        int port_identifier_;
        std::string topic_;
        std::vector<Delegate<void(Port_Event)>> listeners_;
        std::vector<Delegate<void(T)>> consumers_;

        constexpr static vsomeip::service_t SERVICE_ID = 0x1234;
        constexpr static vsomeip::instance_t INSTANCE_ID = 0x5678;
        constexpr static vsomeip::event_t EVENT_ID = 0x0421;
        constexpr static vsomeip::eventgroup_t EVENTGROUP_ID = 0x0001;

        std::shared_ptr<vsomeip::application> application_;
        std::thread application_thread_;
    };
}

#endif