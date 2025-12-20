#include <vsomeip_output_port.h>
#include <output_port.h>
#include <component.h>
#include <periodic_timer_port.h>
#include <blocking_queue.h>
#include <behavior.h>
#include <node.h>

#include <iostream>


class Message 
{
    public:
        Message() = default;

        Message(char* buffer, int size_bytes)
        {
            (void)size_bytes;
            payload = buffer;
        }

        std::string payload;

        char* serialize()
        {
            return payload.data();
        }

        int size_bytes()
        {
            return payload.size();
        }
};

namespace abacos
{
    template<typename TOutput_Port>
    requires Output_Port_Concept<TOutput_Port, Message>
    class Simple_Publisher
            : public Component
    {
        public:
            explicit Simple_Publisher(long period_ms)
                    : periodic_timer_port_(0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::milliseconds(period_ms)).count()),
                    output_port_(1, "vsomeip_topic")
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

                Message message;
                message.payload = "Hello from Cyclone DDS";
                
                output_port_.write(message);
            }

            abacos::Periodic_Timer_Port periodic_timer_port_;
            TOutput_Port output_port_;
            int current_id_ = 0;
    };
}



int main()
{
    abacos::Simple_Publisher<abacos::VSOMEIP_Output_Port<Message>> publisher(500);

    publisher.start();

    while(true);

    return 0;
}