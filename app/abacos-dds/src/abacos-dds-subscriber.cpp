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

#include <fstream>

namespace abacos
{
    template <typename TInput_Port, typename TData>
        requires Input_Port_Concept<TInput_Port, TData>
    class Simple_Subscriber : public Component
    {
    public:
        Simple_Subscriber()
            : input_port_(1, "dds_topic"),
              behavior_(
                  Behavior<
                      Simple_Subscriber,
                      TData,
                      TInput_Port>::template create_behavior<&Simple_Subscriber::receive>(this, &input_port_)),
              output_file_open_(false)
        {
            bind_behavior_to_input_port(
                &input_port_,
                behavior_.create_behavior_delegate());
        }

    private:
        void receive(const TData &message)
        {
            long now_ns =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            const_cast<TData &>(message).abacos_subscriber_time_stamp_ns(now_ns);

            /*const long dds_latency =
                message.dds_subscriber_time_stamp_ns() -
                message.dds_publisher_time_stamp_ns();

            const long abacos_latency =
                message.abacos_subscriber_time_stamp_ns() -
                message.abacos_publisher_time_stamp_ns();

            const long abacos_overhead =
                abacos_latency - dds_latency;

            const long abacos_publisher_overhead =
                message.dds_publisher_time_stamp_ns() -
                message.abacos_publisher_time_stamp_ns();

            const long abacos_subscriber_overhead =
                message.abacos_subscriber_time_stamp_ns() -
                message.dds_subscriber_time_stamp_ns();

            std::cout
                << "message received - payload size: "
                << message.payload().size() << "\n"
                << " - abacos publisher time stamp [ns]: "
                << message.abacos_publisher_time_stamp_ns() << "\n"
                << " - dds publisher time stamp [ns]: "
                << message.dds_publisher_time_stamp_ns() << "\n"
                << " - dds subscriber time stamp [ns]: "
                << message.dds_subscriber_time_stamp_ns() << "\n"
                << " - abacos subscriber time stamp [ns]: "
                << message.abacos_subscriber_time_stamp_ns() << "\n"
                << " - dds latency [ns]: "
                << dds_latency << "\n"
                << " - abacos latency [ns]: "
                << abacos_latency << "\n"
                << " - abacos overhead [ns]: "
                << abacos_overhead << "\n"
                << " - abacos publisher overhead [ns]: "
                << abacos_publisher_overhead << "\n"
                << " - abacos subscriber overhead [ns]: "
                << abacos_subscriber_overhead << "\n"
                /*<< " - abacos relative overhead [%]: "
                << 100.0 * static_cast<double>(abacos_overhead) /
                       static_cast<double>(dds_latency)
                << "\n"
                << std::endl;*/

            if (!output_file_open_)
            {
                std::string file_name = "/app/abacos/app/abacos-dds/output/result-" + std::to_string((long)message.payload().size()) + ".csv";


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
                << message.payload().size() << ","
                << message.abacos_publisher_time_stamp_ns() << ","
                << message.dds_publisher_time_stamp_ns() << ","
                << message.dds_subscriber_time_stamp_ns() << ","
                << message.abacos_subscriber_time_stamp_ns() << "\n";

            output_file_.flush();

            volatile auto id = message.id();
            (void)id;
        }

        TInput_Port input_port_;
        Behavior<Simple_Subscriber, TData, TInput_Port> behavior_;
        std::ofstream output_file_;
        bool output_file_open_;
    };
}

int main()
{
    abacos::Simple_Subscriber<abacos::DDS_Input_Port<Packet::PacketMessage>, Packet::PacketMessage> subscriber;

    subscriber.start();

    while (true)
        ;

    return 0;
}