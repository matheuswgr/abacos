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
        virtual std::vector<Topic_Input_Port_Interface *> intra_node_input_ports() const = 0;
        virtual std::vector<Topic_Output_Port_Interface *> intra_node_output_ports() const = 0;
        virtual void start() = 0;
    };

    class Component : public Component_Interface
    {
    public:
        void handle_port_event(const Port_Event &event)
        {
            event_queue_.push(&event);
        }

        template <typename TPort>
        void bind_behavior_to_input_port(TPort *port,
                                         Behavior_Delegate behavior_delegate)
        {
            if (port_behavior_.find(port->port_identifier()) == port_behavior_.end())
            {
                port->listen(
                    Delegate<void(const Port_Event &)>::template create_delegate<Component,
                                                                                 &Component::handle_port_event>(this));

                port_behavior_.emplace(
                    port->port_identifier(),
                    std::vector<Behavior_Delegate>{behavior_delegate});
            }
            else
            {
                port_behavior_.at(port->port_identifier())
                    .emplace_back(behavior_delegate);
            }
        }

        template <typename T>
        void bind_behavior_to_input_port(Topic_Input_Port<T> *port,
                                         Behavior_Delegate behavior_delegate)
        {
            register_topic_input_port(port);
            bind_behavior_to_input_port<Topic_Input_Port<T>>(port,
                                                             behavior_delegate);
        }

        template <typename TPort>
        void register_output_port(TPort *)
        {
            // no-op
        }

        template <typename T>
        void register_output_port(Topic_Output_Port<T> *port)
        {
            register_topic_output_port(port);
        }

        std::vector<Topic_Input_Port_Interface *> intra_node_input_ports() const override
        {
            return topic_input_ports_;
        }

        std::vector<Topic_Output_Port_Interface *> intra_node_output_ports() const override
        {
            return topic_output_ports_;
        }

        void start() override
        {
            if (!running_)
            {
                running_ = true;
                runner_ = std::thread(&Component::run, this);
            }
        }

    protected:
        explicit Component(int event_queue_capacity = 10)
            : event_queue_(event_queue_capacity)
        {
        }

    private:
        template <typename T>
        void register_topic_input_port(Topic_Input_Port<T> *input_port)
        {
            topic_input_ports_.push_back(
                static_cast<Topic_Input_Port_Interface *>(input_port));
        }

        template <typename T>
        void register_topic_output_port(Topic_Output_Port<T> *output_port)
        {
            topic_output_ports_.push_back(
                static_cast<Topic_Output_Port_Interface *>(output_port));
        }

        void run()
        {
            while (true)
            {
                // 🔹 pop returns a pointer
                const Port_Event *event = event_queue_.pop();

                auto it = port_behavior_.find(event->port_identifier());
                if (it == port_behavior_.end())
                    continue;

                for (auto &handler : it->second)
                    handler();
            }
        }

        bool running_ = false;

        std::vector<Topic_Input_Port_Interface *> topic_input_ports_;
        std::vector<Topic_Output_Port_Interface *> topic_output_ports_;

        std::thread runner_;

        std::map<int, std::vector<Behavior_Delegate>> port_behavior_;

        Blocking_Queue<const Port_Event *> event_queue_;
    };

}

#endif
