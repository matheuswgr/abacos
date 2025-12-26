#ifndef __dds_input_port_h
#define __dds_input_port_h

#include <string>
#include <vector>
#include <iostream>
#include <thread>

#include "delegate.h"
#include "port_event.h"

#include <dds/dds.hpp>

namespace abacos
{
    template<typename T>
    class DDS_Input_Port
    {
    public:
        explicit DDS_Input_Port(int port_identifier, std::string topic)
            : port_identifier_(port_identifier),
              topic_(std::move(topic)),
              event_(port_identifier_),
              participant_(0),
              dds_topic_(participant_, topic_),
              subscriber_(participant_),
              reader_qos_(
                  subscriber_.default_datareader_qos()
                      << dds::core::policy::Reliability::Reliable()
                      << dds::core::policy::History::KeepLast(1)),
              reader_(subscriber_, dds_topic_, reader_qos_),
              runner_(&DDS_Input_Port::run, this)
        {}

        void listen(Delegate<void(const Port_Event&)> listener)
        {
            listeners_.push_back(listener);
        }

        void bind(Delegate<void(const T&)> consumer)
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

        std::vector<Delegate<void(const Port_Event&)>> listeners_;
        std::vector<Delegate<void(const T&)>> consumers_;

        dds::domain::DomainParticipant participant_;
        dds::topic::Topic<T> dds_topic_;
        dds::sub::Subscriber subscriber_;
        dds::sub::qos::DataReaderQos reader_qos_;
        dds::sub::DataReader<T> reader_;

        std::thread runner_;

        void run()
        {
            while (true)
            {
                reader_.wait_for_historical_data(
                    dds::core::Duration(1, 0));

                auto samples = reader_.take();

                for (auto& sample : samples)
                {
                    if (!sample.info().valid())
                        continue;

                    const T& msg = sample.data();

                    const_cast<T &>(msg).dds_subscriber_time_stamp_ns(std::chrono::duration_cast<std::chrono::nanoseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count());

                    for (auto& consumer : consumers_)
                        consumer(msg);

                    for (auto& listener : listeners_)
                        listener(event_);
                }
            }
        }
    };
}


#endif