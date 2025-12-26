#include "../include/dds_output_port.h"
#include "../include/output_port.h"
#include "../include/component.h"
#include "../include/periodic_timer_port.h"
#include "../include/blocking_queue.h"
#include "../include/behavior.h"
#include "../include/topic_input_port.h"
#include "../include/topic_output_port.h"
#include "../include/node.h"

#include <iostream>

#include "packet.hpp"

namespace abacos
{
    template<typename TOutput_Port>
    requires Output_Port_Concept<TOutput_Port, Packet::PacketMessage>
    class Simple_Publisher : public Component
    {
    public:
        Simple_Publisher(long period_ms,
                         std::size_t payload_size_bytes)
            : payload_size_bytes_(payload_size_bytes),
              periodic_timer_port_(
                  0,
                  std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::milliseconds(period_ms)).count()),
              output_port_(1, "dds_topic"),
              behavior_(
                  Behavior<Simple_Publisher>
                      ::template create_behavior<&Simple_Publisher::publish>(this))
        {
            register_output_port(&output_port_);

            bind_behavior_to_input_port(
                &periodic_timer_port_,
                behavior_.create_behavior_delegate());

            // 🔹 Preallocate payload once (deterministic)
            payload_.resize(payload_size_bytes_, 'A');
        }

    private:
        void publish()
        {
            Packet::PacketMessage packet_message;

            packet_message.payload(payload_);
            packet_message.id(current_id_++);

            packet_message.abacos_publisher_time_stamp_ns(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());

            output_port_.write(packet_message);
        }

        std::size_t payload_size_bytes_;
        std::string payload_;

        Behavior<Simple_Publisher> behavior_;
        Periodic_Timer_Port periodic_timer_port_;
        TOutput_Port output_port_;

        int current_id_ = 0;
    };
}



int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <period_ms> <payload_size_bytes>\n";
        return 1;
    }

    long period_ms =
        std::stol(argv[1]);

    std::size_t payload_size_bytes =
        static_cast<std::size_t>(std::stoul(argv[2]));

    abacos::Simple_Publisher<
        abacos::DDS_Output_Port<Packet::PacketMessage>>
        publisher(period_ms, payload_size_bytes);

    publisher.start();

    while (true) {}

    return 0;
}