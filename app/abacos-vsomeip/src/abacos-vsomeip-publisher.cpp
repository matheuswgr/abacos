#include <vsomeip_output_port.h>
#include <output_port.h>
#include <component.h>
#include <periodic_timer_port.h>
#include <blocking_queue.h>
#include <behavior.h>
#include <node.h>
#include <message.h>

#include <iostream>
#include <chrono>
#include <sstream>
#include <cstring>

namespace abacos
{
    template <typename TOutput_Port>
        requires Output_Port_Concept<TOutput_Port, Message>
    class Simple_Publisher : public Component
    {
    public:
        explicit Simple_Publisher(long period_ms)
            : periodic_timer_port_(
                  0,
                  std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::milliseconds(period_ms))
                      .count()),
              output_port_(1, "vsomeip_topic"),
              behavior_(Behavior<Simple_Publisher>
                    ::template create_behavior<&Simple_Publisher::publish>(
                        this))
        {
            register_output_port(&output_port_);

            bind_behavior_to_input_port(
                &periodic_timer_port_,
                behavior_.create_behavior_delegate());
        }

    private:
        void publish()
        {
            Message message;

            message.payload = "Hello from vSomeIP";
            message.payload_size_bytes = message.payload.size();

            message.abacos_publisher_time_stamp_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            output_port_.write(message);
        }

        Behavior<Simple_Publisher> behavior_;
        Periodic_Timer_Port periodic_timer_port_;
        TOutput_Port output_port_;
    };
}

int main()
{
    abacos::Simple_Publisher<
        abacos::VSOMEIP_Output_Port<Message>>
        publisher(500);

    publisher.start();

    while (true)
    {
    }

    return 0;
}
