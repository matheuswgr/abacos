
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

    dds::pub::Publisher publisher(participant);

    dds::pub::qos::DataWriterQos writer_qos =
        publisher.default_datawriter_qos()
            << dds::core::policy::Reliability::Reliable()
            << dds::core::policy::History::KeepLast(1);
            
    dds::pub::DataWriter<Packet::PacketMessage> writer(
        publisher,
        topic,
        writer_qos
    );

    Packet::PacketMessage packet_message;
    packet_message.id(0);

    while (true)
    {
        packet_message.payload("Hello from Cyclone DDS");
        packet_message.id(packet_message.id()+1);

        writer.write(packet_message);

        std::cout << "Published: id=" << packet_message.id() << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}