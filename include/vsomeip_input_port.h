#ifndef __vsomeip_input_port_h
#define __vsomeip_input_port_h

#include <vsomeip/vsomeip.hpp>
#include <thread>

#include <port_event.h>
#include <delegate.h>
#include <serializable.h>
#include <iostream>
#include <chrono>
#include <optional>

namespace abacos
{
    template <typename T>
        requires Serializable<T>
    class VSOMEIP_Input_Port
    {
    public:
        VSOMEIP_Input_Port(int port_identifier, std::string topic)
            : port_identifier_(port_identifier),
              topic_(std::move(topic)),
              event_(port_identifier_)
        {
            application_ =
                vsomeip::runtime::get()->create_application("vsomeip_subscriber");

            application_->init();

            application_->register_message_handler(
                SERVICE_ID,
                INSTANCE_ID,
                EVENT_ID,
                [this](const std::shared_ptr<vsomeip::message> &msg)
                {
                    auto payload = msg->get_payload();

                    current_data_ = T(reinterpret_cast<char *>(payload->get_data()),
                        payload->get_length());

                    current_data_.vsomeip_subscriber_time_stamp_ns =
                        std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count();

                    for (auto &consumer : consumers_)
                        consumer(current_data_);

                    for (auto &listener : listeners_)
                        listener(event_);
                });

            application_->register_state_handler(
                [this](vsomeip::state_type_e state)
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

        void listen(Delegate<void(const Port_Event &)> listener)
        {
            listeners_.push_back(listener);
        }

        void bind(Delegate<void(const T &)> consumer)
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

        Port_Event event_;

        T current_data_;

        std::vector<Delegate<void(const Port_Event &)>> listeners_;
        std::vector<Delegate<void(const T &)>> consumers_;

        constexpr static vsomeip::service_t SERVICE_ID = 0x1234;
        constexpr static vsomeip::instance_t INSTANCE_ID = 0x5678;
        constexpr static vsomeip::event_t EVENT_ID = 0x0421;
        constexpr static vsomeip::eventgroup_t EVENTGROUP_ID = 0x0001;

        std::shared_ptr<vsomeip::application> application_;
        std::thread runner_;

        void run()
        {
            application_->start();
        }
    };

}

#endif