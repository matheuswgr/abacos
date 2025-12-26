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
        Simple_Publisher(std::size_t payload_size_bytes,
                         long period_ms)
            : payload_size_bytes_(payload_size_bytes),
              period_ns_(static_cast<long>(
                  period_ms*1000000)),
              periodic_timer_port_(0, period_ns_),
              output_port_(1, "vsomeip_topic"),
              behavior_(Behavior<Simple_Publisher>
                    ::template create_behavior<&Simple_Publisher::publish>(
                        this))
        {
            register_output_port(&output_port_);

            bind_behavior_to_input_port(
                &periodic_timer_port_,
                behavior_.create_behavior_delegate());

            payload_.resize(payload_size_bytes_, 'A');
        }

    private:
        void publish()
        {
            Message message;

            message.payload = payload_;
            message.payload_size_bytes = payload_.size();

            message.abacos_publisher_time_stamp_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            output_port_.write(message);
        }

        std::size_t payload_size_bytes_;
        long period_ns_;

        std::string payload_;

        Behavior<Simple_Publisher> behavior_;
        Periodic_Timer_Port periodic_timer_port_;
        TOutput_Port output_port_;
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
        abacos::VSOMEIP_Output_Port<Message>>
        publisher(payload_size_bytes, period_ms);

    publisher.start();

    while (true) {}

    return 0;
}
