
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

class Periodic_Data_Source
        : public abacos::Component
{
    public:
        explicit Periodic_Data_Source(long period_ms)
                : periodic_timer_port_(0, std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::milliseconds(period_ms)).count()),
                  output_port_(1, "samples")
        {
            register_output_port(&output_port_);

            bind_behavior_to_input_port(&periodic_timer_port_,
                                        abacos::Behavior<Periodic_Data_Source>::create_behavior_delegate<&Periodic_Data_Source::produce_data>(
                                                this));
        }

    private:
        void produce_data()
        {
            output_port_.write(value_);
        }

        abacos::Periodic_Timer_Port periodic_timer_port_;
        abacos::Topic_Output_Port<int> output_port_;
        int value_ = 1;
};

class Inverting_Integrator
        : public abacos::Component
{
    public:
        explicit Inverting_Integrator()
                : input_port_(1, "samples")
        {
            bind_behavior_to_input_port(&input_port_,
                                        abacos::Behavior<Inverting_Integrator, int, abacos::Topic_Input_Port<int>>::create_behavior_delegate<&Inverting_Integrator::integrate_inverting>(
                                                this, &input_port_));
            bind_behavior_to_input_port(&input_port_,
                                        abacos::Behavior<Inverting_Integrator, int, abacos::Topic_Input_Port<int>>::create_behavior_delegate<&Inverting_Integrator::integrate>(
                                                this, &input_port_));
        }

        void integrate(int sample)
        {
            integral_ += sample;
            std::cout << "Inverting_Integrator::integrate - value: " << integral_ << std::endl;
        }

        void integrate_inverting(int sample)
        {
            inverted_integral_ -= sample;
            std::cout << "Inverting_Integrator::integrate_inverting - value: " << inverted_integral_ << std::endl;
        }

    private:
        long integral_ = 0;
        long inverted_integral_ = 0;

        abacos::Topic_Input_Port<int> input_port_;
};

/*
 * 1. Rename topic input/output ports to intra_node_input_port and intra_node_output_port
 * 2. Make things thread safe
 * 3. Graceful shutdown of all threads
 * 4. Remove memory leaks
 * 5. Periodic timer port runs every time and has negative sleep times
 * 6. Avoid data races in initialization
 * */

/*
 * For phase 2:
 *
 * DDS Ports
 * DDS Ports interoperability with ROS2
 * SOME/IP Ports
 *
 * Measure overhead and you have a publication!
 * */

/*
 * Ports currently have too many responsibilities, they should expose interfaces, not produce data, so data producers
 * should be bound in a different, more generic fashion. Keep in mind that if a data source produces data of the correct type
 * it should be able to write to the port, even if there are two different sources.
 *
 * A general refactor would be fine
 *
 * Use concepts to constrain templates whenever possible
 * */

int main()
{
    Periodic_Data_Source periodic_data_source(500);
    Inverting_Integrator inverting_integrator;

    abacos::Node simple_node;

    simple_node.add_component(&periodic_data_source);
    simple_node.add_component(&inverting_integrator);

    simple_node.start();

    return 0;
}