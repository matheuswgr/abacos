#ifndef __input_port_h
#define __input_port_h

#include <concepts>

#include "port_event.h"
#include "delegate.h"

namespace abacos
{
    template<typename TComponent_Input_Port, typename T>
    concept Input_Port_Concept =
    requires(TComponent_Input_Port component_input_port,          int id,
         std::string topic, Delegate<void(Port_Event)> listener, Delegate<void(T)> consumer)
    {
        TComponent_Input_Port{id, topic};
        { component_input_port.listen(listener) } -> std::same_as<void>;
        { component_input_port.bind(consumer) } -> std::same_as<void>;
    };
}

#endif
