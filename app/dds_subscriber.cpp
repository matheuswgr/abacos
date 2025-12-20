
#include "../include/component.h"
#include "../include/periodic_timer_port.h"
#include "../include/blocking_queue.h"
#include "../include/behavior.h"
#include "../include/topic_input_port.h"
#include "../include/topic_output_port.h"
#include "../include/node.h"

#include <string>
#include <utility>
#include <vector>

#include <dds/dds.hpp>
#include "packet.hpp"

int main()
{
    dds::domain::DomainParticipant participant(0);

    dds::topic::Topic<Packet::PacketMessage> topic(
        participant,
        "SimpleTopic"
    );

    dds::sub::Subscriber subscriber(participant);

    dds::sub::qos::DataReaderQos reader_qos =
            subscriber.default_datareader_qos()
                << dds::core::policy::Reliability::Reliable()
                << dds::core::policy::History::KeepLast(10);
            
    dds::sub::DataReader<Packet::PacketMessage> reader(
            subscriber,
            topic,
            reader_qos
        );

    Packet::PacketMessage packet_message;
    packet_message.id(0);

    std::cout << "Subscriber started, waiting for data..." << std::endl;

    while (true)
    {
        reader.wait_for_historical_data(dds::core::Duration(1, 0));

        auto samples = reader.take();

        for (const auto &sample : samples)
        {
            if (sample.info().valid())
            {
                const auto &msg = sample.data();
                std::cout
                    << "Received: id=" << msg.id()
                    << ", payload=\"" << msg.payload() << "\""
                    << std::endl;
            }
        }
    }

    return 0;
}