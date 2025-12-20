#ifndef __delegate_h
#define __delegate_h

namespace abacos
{

    template<typename TSignature>
    class Delegate;

    template<typename R, typename ...Args>
    class Delegate<R(Args...)>
    {
        public:
            R operator()(Args... args) const
            {
                return (*stub_)(object_, args...);
            }

            template<typename T>
            static Delegate create_delegate(T *object)
            {
                return Delegate(object, &create_functor_stub < T > );
            }

            template<typename T, R(T::*Method)(Args...)>
            static Delegate create_delegate(T *object)
            {
                return Delegate(object, &create_method_stub < T, Method > );
            }

        private:
            using stub_type = R(*)(void *, Args...);

            void *object_;
            stub_type stub_;

            Delegate(void *object, stub_type stub)
                    : object_(object), stub_(stub)
            {}

            template<typename T, R(T::*Method)(Args...)>
            static R create_method_stub(void *object, Args... args)
            {
                T *typed_object = static_cast<T *>(object);
                return (typed_object->*Method)(args...);
            }

            template<typename T>
            static R create_functor_stub(void *object, Args... args)
            {
                T *typed_object = static_cast<T *>(object);
                return (*typed_object)(args...);
            }
    };

    template<>
    class Delegate<void()>
    {
        public:
            void operator()() const
            {
                return (*stub_)(object_);
            }

            template<typename T>
            static Delegate create_delegate(T *object)
            {
                return Delegate(object, &create_functor_stub<T>);
            }

            template<typename T, void(T::*Method)()>
            static Delegate create_delegate(T *object)
            {
                return Delegate(object, &create_method_stub<T, Method>);
            }

        private:
            using stub_type = void (*)(void *);

            void *object_;
            stub_type stub_;

            Delegate(void *object, stub_type stub)
                    : object_(object), stub_(stub)
            {}

            template<typename T, void(T::*Method)()>
            static void create_method_stub(void *object)
            {
                T *typed_object = static_cast<T *>(object);
                return (typed_object->*Method)();
            }

            template<typename T>
            static void create_functor_stub(void *object)
            {
                T *typed_object = static_cast<T *>(object);
                return (*typed_object)();
            }
    };
}
#endif
