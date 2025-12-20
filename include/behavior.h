#ifndef __behavior_h
#define __behavior_h

#include "delegate.h"
#include "input_port.h"
#include "blocking_queue.h"

namespace abacos
{
    using Behavior_Delegate = Delegate<void()>;

    template<typename ...TSignature>
    class Behavior;

    template<typename T, typename TIn, Input_Port_Concept <TIn> TInput_Port>
    class Behavior<T, TIn, TInput_Port>
    {
        public:
            template<void(T::*TMethod)(TIn)>
            static Behavior create_behavior(T *object, TInput_Port *input_port)
            {
                return Behavior(object, input_port, &create_method_forward < TMethod > );
            }

            template<void(T::*TMethod)(TIn)>
            static Behavior_Delegate create_behavior_delegate(T *object, TInput_Port *input_port)
            {
                Behavior *behavior = new Behavior(object, input_port, &create_method_forward < TMethod > );

                return Behavior_Delegate::create_delegate<Behavior<T, TIn, TInput_Port>>(behavior);
            }

            Behavior_Delegate create_behavior_delegate()
            {
                return Behavior_Delegate::create_delegate<Behavior<T, TIn>>(this);
            }

            void ingest(TIn input)
            {
                input_queue_.push(input);
            }

            void operator()()
            {
                return (*method_forward_)(object_, &input_queue_);
            }

        private:
            using method_forward_type = void (*)(T *,Blocking_Queue<TIn>*);

            T *object_;
            method_forward_type method_forward_;
            Blocking_Queue<TIn> input_queue_;

            Behavior(T *object, TInput_Port *input_port, method_forward_type method_forward, int queue_capacity = 10)
                    : object_(object), input_queue_(queue_capacity), method_forward_(method_forward)
            {
                input_port->bind(Delegate<void(TIn)>::template create_delegate<Behavior<T, TIn, TInput_Port>, &Behavior<T, TIn, TInput_Port>::ingest>(this));
            }

            template<void(T::*TMethod)(TIn)>
            static void create_method_forward(T *object, Blocking_Queue<TIn> *input_queue_)
            {
                TIn input = input_queue_->pop();
                return (object->*TMethod)(input);
            };
    };

    template<typename T>
    class Behavior<T>
    {
        public:
            template<void(T::*TMethod)()>
            static Behavior create_behavior(T *object)
            {
                return Behavior(object, &create_method_forward<TMethod>);
            }

            template<void(T::*TMethod)()>
            static Behavior_Delegate create_behavior_delegate(T *object)
            {
                Behavior *behavior = new Behavior(object, &create_method_forward<TMethod>);

                return Behavior_Delegate::create_delegate<Behavior<T>>(behavior);
            }

            Behavior_Delegate create_behavior_delegate()
            {
                return Behavior_Delegate::create_delegate<Behavior<T>>(this);
            }

            void operator()()
            {
                return (*method_forward_)(object_);
            }

        private:
            using method_forward_type = void (*)(T *);

            T *object_;
            method_forward_type method_forward_;

            Behavior(T *object, method_forward_type method_forward)
                    : object_(object), method_forward_(method_forward)
            {}

            template<void(T::*TMethod)()>
            static void create_method_forward(T *object)
            {
                return (object->*TMethod)();
            };
    };
}

#endif
