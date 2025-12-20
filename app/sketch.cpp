#include <iostream>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <concepts>

namespace abacos
{

    class Node
    {

    };

    template<typename T>
    class Blocking_Queue
    {
        private:
            std::mutex _sync;

            std::queue<T> _queue;

            std::condition_variable _queue_not_empty;
            std::condition_variable _queue_not_full;

            unsigned int _queue_length;
            const unsigned int _queue_capacity;

        public:
            explicit Blocking_Queue(int capacity) : _queue_capacity(capacity)
            {
                _queue_length = 0;
            }

            void push(T item)
            {
                std::unique_lock<std::mutex> lock(_sync);

                _queue_not_full.wait(lock, [this] { return _queue_length < _queue_capacity ; });

                _queue.push(item);

                _queue_length++;

                _queue_not_empty.notify_one();
            }

            T pop()
            {
                std::unique_lock<std::mutex> lock(_sync);

                _queue_not_empty.wait(lock, [this] { return !_queue.empty(); });

                T item = _queue.front();

                _queue.pop();

                _queue_length--;

                _queue_not_full.notify_one();

                return item;
            }
    };

    template<typename TNotifiable>
    concept NotifiableConcept = requires(TNotifiable notifiable)
    {
        { notifiable.notify() };
    };

    template<typename TComponent_Output_Port, typename T>
    concept Component_Output_Port_Concept = requires(TComponent_Output_Port component_output_port, T data)
    {
        { component_output_port.write(data) };
    };

    template<typename TComponent_Input_Port, typename T>
    concept Component_Input_Port_Concept = requires(TComponent_Input_Port component_input_port, T data)
    {
        { component_input_port.read() } -> std::same_as<T>;
    };

    //    template<typename T, NotifiableConcept TNotifiable>
    template<typename T>
    class Blocking_Queue_Port
    {
        private:
            Blocking_Queue<T> _queue;

        public:
            explicit Blocking_Queue_Port(long queue_capacity)
                    : _queue(queue_capacity)
            {}

            void write(T data)
            {
                _queue.push(data);
            }

            T read()
            {
                return _queue.pop();
            }
    };

    template<typename T>
    class Component_Input_Port
    {
        private:
            std::function<void(T)> _handler;

        public:
            Component_Input_Port()
                    : _handler([](T)
                               {})
            {}

        public:
            void bind_behavior(std::function<void(T)> handler)
            {
                _handler = std::move(handler);
            }

            void push(T data)
            {
                _handler(data);
            }
    };

    template<typename T>
    class Component_Output_Port
    {
        private:
            T _value;

        public:
            void write(T value)
            {
                _value = value;
            }
    };

    class Timer_Component_Input_Port
    {
        private:
            long _period_ms = 0;
            std::function<void(void)> _handler;

        public:
            explicit Timer_Component_Input_Port(long period_ms)
                    : _period_ms(period_ms), _handler([]()
                                                      {})
            {}

        public:
            void bind_behavior(std::function<void(void)> handler)
            {
                _handler = std::move(handler);
            }

            void push()
            {
                _handler();
            }

            long period_ms()
            {
                return _period_ms;
            }
    };

    class Producer_Component
    {
        private:
            int _publish_counter = 0;

            Component_Output_Port<int> _output_port;
            Timer_Component_Input_Port _timer_component_input_port = Timer_Component_Input_Port(100);

            void handle_timer()
            {
                _publish_counter++;
                _output_port.write(_publish_counter);

                if (_publish_counter == std::numeric_limits<int>::max())
                    _publish_counter = 0;
            }

        public:
            Producer_Component()
            {
                _timer_component_input_port.bind_behavior([this]()
                                                          {
                                                              this->handle_timer();
                                                          });
            }

            Timer_Component_Input_Port& get_timer_ports()
            {
                return _timer_component_input_port;
            }
    };

    class Producer_Component_Executor
    {
        private:
            Producer_Component producer_component;
            std::thread _executor_thread;

        public:
            void execute()
            {
                while(true)
                {
                    std::cout << "producing..." << std::endl;

                    producer_component.get_timer_ports().push();
                    std::this_thread::sleep_for(std::chrono::milliseconds(producer_component.get_timer_ports().period_ms()));
                }
            }

            void start()
            {
                this->_executor_thread = std::thread(&Producer_Component_Executor::execute, this);
            }

            void join()
            {
                this->_executor_thread.join();
            }
    };

    class Consumer_Component
    {
        private:
            Component_Input_Port<int> _input_port;

            void handle_input(int value)
            {
                std::cout << value << std::endl;
            }

        public:
            Consumer_Component()
            {
                _input_port.bind_behavior([this](int value)
                                          {
                                              this->handle_input(value);
                                          });
            }
    };
}

class Test_Class
{
    private:
        int _counter = 0;

    public:
        int test_behavior(int x)
        {
            _counter = _counter + x;
            return _counter;
        }
};

template<typename T>
class Simple_Input_Port
{
    private:
        T _value;

    public:
        explicit Simple_Input_Port(T value)
                : _value(value)
        {}

        T read()
        {
            return _value;
        }
};

template<typename T>
class Simple_Output_Port
{
    public:
        Simple_Output_Port() = default;

        void write(T value)
        {
            std::cout << value << std::endl;
        }
};

class Behavior
{
    private:
        std::function<void()> _behavior_map;

    public:
        template<typename TIn, typename TOut, typename F>
        Behavior(Simple_Input_Port<TIn>& input_port, Simple_Output_Port<TOut>& output_port, F&& map)
        {
            _behavior_map = [input_port, output_port, map]()
            {
                output_port.write(std::invoke(map, input_port.read()));
            };
        }
};

class Component
{
    private:
        std::vector<Behavior> _behaviors;

        Simple_Input_Port<int> _input_port;
        Simple_Output_Port<int> _output_port;

        int _integrator_state = 0;
        int _negative_integrator_state = 0;

    private:
        template<typename TIn, typename TOut, typename F>
        auto make_behavior(F&& f, Simple_Input_Port<TIn> input_port, Simple_Output_Port<TOut> output_port)
        {
            using TFunction = std::decay_t<F>;
            return Behavior<TIn, TOut, TFunction>(input_port, output_port, std::forward<F>(f));
        }

    public:
        Component()
                : _input_port(1)
        {
            Behavior<int, int

            _behaviors.push_back(Behavior<>())
        }

        int integrate(int x)
        {
            _integrator_state = _integrator_state + x;
            return _integrator_state;
        }

        int negative_integrate(int x)
        {
            _negative_integrator_state = _negative_integrator_state - x;
            return _negative_integrator_state;
        }
};

/*template <typename TIn, typename TOut, typename F>
class Behavior
{
    private:
        std::decay_t<F> _behavior_map;
        Simple_Input_Port<TIn> _input_port;
        Simple_Output_Port<TOut> _output_port;

    public:
        template<typename G>
        Behavior(G&& g, Simple_Input_Port<TIn> input_port, Simple_Output_Port<TOut> output_port)
            : _behavior_map(std::forward<G>(g)), _input_port(input_port), _output_port(output_port)
        {}

        void execute_behavior()
        {
            _output_port.write(std::invoke(_behavior_map, _input_port.read()));
        }
};*/

/*template <typename TOut, typename TIn, typename F>
auto make_behavior(F&& f, Simple_Input_Port<TIn> input_port, Simple_Output_Port<TOut> output_port) {
    using TFunction = std::decay_t<F>;
    return Behavior<TOut, TIn, TFunction>(std::forward<F>(f), input_port, output_port);
}*/


int main()
{
    // port receives data
    // port notifies component about arrival
    // component pops event queue
    // component finds behavior that processes the event
    // behavior reads from port executes and writes to port

    Test_Class test_class;
    Simple_Input_Port<int> input_port(1);
    Simple_Output_Port<int> output_port;

    auto behavior = make_behavior<int, int>([&](float x){return test_class.test_behavior(x);}, input_port, output_port);

    std::function<void(void)> callable = behavior.get_callable();
    callable();
    callable();
    callable();

    /*abacos::Producer_Component producer_component;

    abacos::Producer_Component_Executor producer_component_executor;
    producer_component_executor.start();
    producer_component_executor.join();*/
    return 0;
}