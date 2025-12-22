#ifndef __component_h
#define __component_h

#include <thread>
#include <map>

#include "port_event.h"
#include "behavior.h"
#include "delegate.h"
#include "blocking_queue.h"
#include "topic_input_port.h"
#include "topic_output_port.h"

#include <iostream>

namespace abacos
{
    class Component_Interface
    {
        public:
            virtual std::vector<Topic_Input_Port_Interface*> intra_node_input_ports() const = 0;
            virtual std::vector<Topic_Output_Port_Interface*> intra_node_output_ports() const = 0;
            virtual void start() = 0;
    };

    class Component
            : public Component_Interface
    {
        public:
            void handle_port_event(Port_Event event)
            {
                std::cout << "Component::handle_port_event - Received event from port: " << event.port_identifier()
                          << std::endl;
                event_queue_.push(event);
            }

            template<typename TPort>
            void bind_behavior_to_input_port(TPort *port, Behavior_Delegate behavior_delegate)
            {
                if (port_behavior_.end() == port_behavior_.find(port->port_identifier()))
                {
                    port->listen(Delegate<void(Port_Event)>::create_delegate<Component, &Component::handle_port_event>(this));

                    port_behavior_.emplace(port->port_identifier(),
                                           std::move(std::vector<Behavior_Delegate>{behavior_delegate}));
                } else
                {
                    port_behavior_.at(port->port_identifier()).emplace_back(behavior_delegate);
                }
            }

            template<typename T>
            void bind_behavior_to_input_port(Topic_Input_Port<T>* port, Behavior_Delegate behavior_delegate)
            {
                register_topic_input_port<T>(port);

                if (port_behavior_.end() == port_behavior_.find(port->port_identifier()))
                {
                    port->listen(Delegate<void(Port_Event)>::create_delegate<Component, &Component::handle_port_event>(this));

                    port_behavior_.emplace(port->port_identifier(),
                                           std::move(std::vector<Behavior_Delegate>{behavior_delegate}));
                } else
                {
                    port_behavior_.at(port->port_identifier()).emplace_back(behavior_delegate);
                }
            }

            template<typename TPort>
            void register_output_port(TPort *port)
            {
                (void)port;
                return;
            }

            template<typename T>
            void register_output_port(Topic_Output_Port<T>* port)
            {
                register_topic_output_port<T>(port);
            }

            std::vector<Topic_Input_Port_Interface*> intra_node_input_ports()  const override
            {
                return topic_input_ports;
            }

            std::vector<Topic_Output_Port_Interface*> intra_node_output_ports() const override
            {
                return topic_output_ports;
            }

            void start() override
            {
                if (!running)
                {
                    runner_ = std::thread(&Component::run, this);
                    running = true;
                }
            }

        protected:
            explicit Component(int event_queue_capacity = 10)
                    : event_queue_(event_queue_capacity)
            {}

        private:

            template<typename T>
            void register_topic_input_port(Topic_Input_Port<T>* input_port)
            {
                topic_input_ports.push_back(static_cast<Topic_Input_Port_Interface*>(input_port));
            }

            template<typename T>
            void register_topic_output_port(Topic_Output_Port<T>* output_port)
            {
                topic_output_ports.push_back(static_cast<Topic_Output_Port_Interface*>(output_port));
            }

            void run()
            {
                while (true)
                {
                    Port_Event event = event_queue_.pop();

                    for (Behavior_Delegate handler: port_behavior_.at(event.port_identifier()))
                        handler();
                }
            }

            bool running = false;

            std::vector<Topic_Input_Port_Interface*> topic_input_ports;
            std::vector<Topic_Output_Port_Interface*> topic_output_ports;

            std::thread runner_;
            std::map<int, std::vector<Behavior_Delegate>> port_behavior_;
            Blocking_Queue<Port_Event> event_queue_;
    };
}

#endif
