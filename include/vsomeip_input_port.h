#ifndef __vsomeip_input_port_h
#define __vsomeip_input_port_h

#include <vsomeip/vsomeip.hpp>
#include <thread>

#include <port_event.h>
#include <delegate.h>
#include <serializable.h>
#include <iostream>
#include <chrono>

namespace abacos
{
    template <typename T>
        requires Serializable<T>
    class VSOMEIP_Input_Port
    {
    public:
        VSOMEIP_Input_Port(int port_identifier, std::string topic)
            : port_identifier_(port_identifier), topic_(topic)
        {
            application_ = vsomeip::runtime::get()->create_application("vsomeip_subscriber");

            application_->init();

            application_->register_message_handler(
                SERVICE_ID,
                INSTANCE_ID,
                EVENT_ID,
                [this](const std::shared_ptr<vsomeip::message> &msg)
                {
                    long vsomeip_subscriber_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                    std::shared_ptr<vsomeip::payload> payload = msg->get_payload();

                    long pre_subscriber_deserialization_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                    T data(reinterpret_cast<char *>(payload->get_data()), payload->get_length());

                    data.vsomeip_subscriber_time_stamp_ns = vsomeip_subscriber_time_stamp_ns;
                    data.pre_subscriber_deserialization_time_stamp_ns = pre_subscriber_deserialization_time_stamp_ns;
                    data.post_subscriber_deserialization_time_stamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                    std::cout << "Received: " << data.serialize() << std::endl;

                    for (Delegate<void(T)> consumer : consumers_)
                        consumer(data);

                    for (Delegate<void(Port_Event)> listener : listeners_)
                        listener(Port_Event(port_identifier_));
                });

            application_->register_state_handler(
                [&](vsomeip::state_type_e state)
                {
                    if (state == vsomeip::state_type_e::ST_REGISTERED)
                    {
                        application_->request_service(SERVICE_ID, INSTANCE_ID);

                        std::set<vsomeip::eventgroup_t> groups{EVENTGROUP_ID};

                        application_->request_event(
                            SERVICE_ID,
                            INSTANCE_ID,
                            EVENT_ID,
                            groups,
                            vsomeip::event_type_e::ET_EVENT);

                        application_->subscribe(
                            SERVICE_ID,
                            INSTANCE_ID,
                            EVENTGROUP_ID);
                    }
                });

            runner_ = std::thread(&VSOMEIP_Input_Port::run, this);
        }

        void listen(Delegate<void(Port_Event)> listener)
        {
            listeners_.push_back(listener);
        }

        void bind(Delegate<void(T)> consumer)
        {
            consumers_.push_back(consumer);
        }

        int port_identifier() const
        {
            return port_identifier_;
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
        std::thread runner_;

        void run()
        {
            application_->start();
            std::cout << "quitting" << std::endl;
        }
    };
}

#endif