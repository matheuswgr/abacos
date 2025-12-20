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
    class Simple_Publisher
            : public Component
    {
        public:
            explicit Simple_Publisher(long period_ms)
                    : periodic_timer_port_(0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::milliseconds(period_ms)).count()),
                    output_port_(1, "dds_topic")
            {
                register_output_port(&output_port_);

                bind_behavior_to_input_port(&periodic_timer_port_,
                                            abacos::Behavior<Simple_Publisher>:: template create_behavior_delegate<&Simple_Publisher::publish>(
                                                    this));
            }

        private:
            void publish()
            {
                std::cout << "writting data" << std::endl;

                Packet::PacketMessage packet_message;
                packet_message.payload("Hello from Cyclone DDS");
                packet_message.id(current_id_++);
                
                output_port_.write(packet_message);
            }

            abacos::Periodic_Timer_Port periodic_timer_port_;
            TOutput_Port output_port_;
            int current_id_ = 0;
    };
}



int main()
{
    abacos::Simple_Publisher<abacos::DDS_Output_Port<Packet::PacketMessage>> publisher(500);

    publisher.start();

    while(true);

    return 0;
}