#include <vsomeip_input_port.h>
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
#include <fstream>

namespace abacos
{
    template <typename TInput_Port, typename TData>
        requires Input_Port_Concept<TInput_Port, TData>
    class Simple_Subscriber : public Component
    {
    public:
        Simple_Subscriber()
            : input_port_(1, "vsomeip_topic"),
            behavior_(Behavior<Simple_Subscriber, TData, TInput_Port>
                    ::template create_behavior<&Simple_Subscriber::receive>(
                        this, &input_port_)),
                        output_file_open_(false)
        {
            bind_behavior_to_input_port(
                &input_port_,
                behavior_.create_behavior_delegate());
        }

    private:
        void receive(const TData &message)
        {
            const auto now_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            const_cast<TData &>(message).abacos_subscriber_time_stamp_ns = now_ns;

            /*const long vsomeip_latency =
                message.vsomeip_subscriber_time_stamp_ns -
                message.vsomeip_publisher_time_stamp_ns;

            const long abacos_latency =
                message.abacos_subscriber_time_stamp_ns -
                message.abacos_publisher_time_stamp_ns;

            const long abacos_overhead =
                abacos_latency - vsomeip_latency;

            const long abacos_publisher_overhead =
                message.vsomeip_publisher_time_stamp_ns -
                message.abacos_publisher_time_stamp_ns;

            const long abacos_subscriber_overhead =
                message.abacos_subscriber_time_stamp_ns -
                message.vsomeip_subscriber_time_stamp_ns;

            std::cout
                << "message received - payload size: "
                << message.payload_size_bytes << "\n"
                << " - abacos publisher time stamp [ns]: "
                << message.abacos_publisher_time_stamp_ns << "\n"
                << " - vsomeip publisher time stamp [ns]: "
                << message.vsomeip_publisher_time_stamp_ns << "\n"
                << " - vsomeip subscriber time stamp [ns]: "
                << message.vsomeip_subscriber_time_stamp_ns << "\n"
                << " - abacos subscriber time stamp [ns]: "
                << message.abacos_subscriber_time_stamp_ns << "\n"
                << " - vsomeip latency [ns]: "
                << vsomeip_latency << "\n"
                << " - abacos latency [ns]: "
                << abacos_latency << "\n"
                << " - abacos overhead [ns]: "
                << abacos_overhead << "\n"
                << " - abacos publisher overhead [ns]: "
                << abacos_publisher_overhead << "\n"
                << " - abacos subscriber overhead [ns]: "
                << abacos_subscriber_overhead << "\n"
                << " - abacos relative overhead [%]: "
                << 100.0 * static_cast<double>(abacos_overhead) /
                       static_cast<double>(vsomeip_latency)
                << "\n"
                << std::endl;*/

             if (!output_file_open_)
            {
                std::string file_name = "/app/abacos/app/abacos-vsomeip/output/result-" + std::to_string(message.payload_size_bytes) + ".csv";

                output_file_open_ = true;
                output_file_ = std::ofstream(file_name);

                if (!output_file_.is_open()) 
                {
                    std::cerr << "ERROR: Failed to open " << file_name << std::endl;
                    return;
                }

                output_file_ << "payload_size" << ","
                               << "abacos_publisher_time_stamp_ns" << ","
                               << "dds_publisher_time_stamp_ns" << ","
                               << "dds_subscriber_time_stamp_ns" << ","
                               << "abacos_subscriber_time_stamp_ns" << "\n";
            }

            output_file_
                << message.payload_size_bytes << ","
                << message.abacos_publisher_time_stamp_ns << ","
                << message.vsomeip_publisher_time_stamp_ns << ","
                << message.vsomeip_subscriber_time_stamp_ns << ","
                << message.abacos_subscriber_time_stamp_ns << "\n";

            output_file_.flush();
        }

        TInput_Port input_port_;
        Behavior<Simple_Subscriber, TData, TInput_Port> behavior_;
        std::ofstream output_file_;
        bool output_file_open_;
    };
}

int main()
{
    abacos::Simple_Subscriber<
        abacos::VSOMEIP_Input_Port<Message>,
        Message
    > subscriber;

    subscriber.start();

    while (true) {}

    return 0;
}
