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
                    : port_identifier_(port_identifier), topic_(std::move(topic)), 
                    participant_(0),
                    dds_topic_(participant_, topic_),
                    subscriber_(participant_),
                    reader_qos_(subscriber_.default_datareader_qos()
                                    << dds::core::policy::Reliability::Reliable()
                                    << dds::core::policy::History::KeepLast(1)),
                    reader_(subscriber_, dds_topic_, reader_qos_), runner_(std::thread(&DDS_Input_Port::run, this))
            {}

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

            dds::domain::DomainParticipant participant_;
            dds::topic::Topic<T> dds_topic_;
            dds::sub::Subscriber subscriber_;
            dds::sub::qos::DataReaderQos reader_qos_;
            dds::sub::DataReader<T> reader_;

            std::thread runner_;

            void run()
            {
                std::cout << "running" << std::endl;

                while (true)
                {
                    reader_.wait_for_historical_data(dds::core::Duration(1, 0));

                    auto samples = reader_.take();

                    for (const auto &sample : samples)
                    {
                        if (sample.info().valid())
                        {
                            const auto &msg = sample.data();

                            for (Delegate<void(T)> consumer : consumers_)
                                consumer(msg);

                            for (Delegate<void(Port_Event)> listener: listeners_)
                                listener(Port_Event(port_identifier_));
                        }
                    }
                }
            }

    };
}

#endif