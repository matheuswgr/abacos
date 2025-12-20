#include "../include/dds_input_port.h"
#include "../include/input_port.h"
#include "../include/component.h"
#include "../include/periodic_timer_port.h"
#include "../include/blocking_queue.h"
#include "../include/behavior.h"
#include "../include/topic_input_port.h"
#include "../include/topic_output_port.h"
#include "../include/node.h"

#include "packet.hpp"

namespace abacos
{
    template<typename TInput_Port, typename TData>
    requires Input_Port_Concept<TInput_Port, TData>
    class Simple_Subscriber
            : public Component
    {
        public:
            explicit Simple_Subscriber()
                    : input_port_(1, "dds_topic")
            {
                bind_behavior_to_input_port(&input_port_,
                                            Behavior<Simple_Subscriber, TData, TInput_Port>:: template create_behavior_delegate<&Simple_Subscriber::receive>(
                                                    this, &input_port_));
            }

            void receive(Packet::PacketMessage message)
            {

                std::cout << "Received: id=" << message.id()
                        << ", payload=\"" << message.payload() << "\""
                        << std::endl;
            }

        private:
            TInput_Port input_port_;
    };
}



int main()
{
    abacos::Simple_Subscriber<abacos::DDS_Input_Port<Packet::PacketMessage>, Packet::PacketMessage> subscriber;

    subscriber.start();


    while(true);

    return 0;
}