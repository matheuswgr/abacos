#ifndef __node_h
#define __node_h

#include <map>
#include <vector>

#include "component.h"

namespace abacos
{
    class Node
    {
        public:
            void add_component(Component_Interface *component)
            {
                components_.emplace_back(component);
            }

            void start()
            {
                wire_components();
                start_components();

                while(true);
            }

        private:
            std::vector<Component_Interface*> components_;

            void start_components()
            {
                for (Component_Interface* component : components_)
                    component->start();
            }

            void wire_components()
            {
                std::map<std::string, std::vector<Topic_Input_Port_Interface*>> consumers;
                std::map<std::string, std::vector<Topic_Output_Port_Interface*>> producers;

                for (Component_Interface* component : components_)
                {
                    const std::vector<Topic_Input_Port_Interface *> component_input_ports = component->intra_node_input_ports();
                    const std::vector<Topic_Output_Port_Interface *> component_output_ports = component->intra_node_output_ports();

                    for (Topic_Input_Port_Interface* input_port : component_input_ports)
                    {
                        if (consumers.end() == consumers.find(input_port->topic()))
                        {
                            consumers.emplace(input_port->topic(), std::move(std::vector<Topic_Input_Port_Interface *>{input_port}));
                        } else
                        {
                            consumers.at(input_port->topic()).emplace_back(input_port);
                        }
                    }

                    for (Topic_Output_Port_Interface* output_port : component_output_ports)
                    {
                        if (producers.end() == producers.find(output_port->topic()))
                        {
                            producers.emplace(output_port->topic(), std::move(std::vector<Topic_Output_Port_Interface *>{output_port}));
                        } else
                        {
                            producers.at(output_port->topic()).emplace_back(output_port);
                        }
                    }
                }

                for (std::pair<std::string, std::vector<Topic_Output_Port_Interface*>> topic_producers : producers)
                {
                    std::string topic = topic_producers.first;

                    for (Topic_Output_Port_Interface* topic_producer : topic_producers.second)
                    {
                        for (Topic_Input_Port_Interface* topic_consumer : consumers.at(topic))
                        {
                            topic_producer->subscribe(topic_consumer);
                        }
                    }
                }
            }
    };
}

#endif
