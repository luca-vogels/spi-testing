#include <iostream>
#include <functional>
#include <tuple>
#include <string>
#include <chrono>


// no dynamic args
template<typename Callback>
struct CallbackAndNoArgs {
    Callback callback;

    // Constructor
    CallbackAndNoArgs(Callback cb)
        : callback(cb) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    void execute(size_t mandatoryArg) {
        callback(mandatoryArg);
    }
};


// std::tuple_cat
template<typename Callback, typename... Args>
struct CallbackAndArgsTupleCat {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackAndArgsTupleCat(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    void execute(size_t mandatoryArg) {
        auto fullArgs = std::tuple_cat(std::make_tuple(mandatoryArg), args);
        std::apply(callback, fullArgs);
    }
};


// std::index_sequence
template<typename Callback, typename... Args>
struct CallbackAndArgsIndexSequence {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackAndArgsIndexSequence(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Helper function to invoke the callback with the mandatory size_t and additional args
    template <std::size_t... I>
    void invoke(std::index_sequence<I...>, size_t mandatoryArg) {
        callback(mandatoryArg, std::get<I>(args)...);
    }

    // Execute the stored callback with its arguments, including the mandatory size_t
    void execute(size_t mandatoryArg) {
        invoke(std::index_sequence_for<Args...>{}, mandatoryArg);
    }
};




// Example functions to be used as callbacks
void exampleFunction1(size_t a) {
    size_t c = a + a;
    (void)c;
}

// Example functions to be used as callbacks
void exampleFunction2(size_t a, std::string& b) {
    size_t c = a + b.size();
    (void)c;
}




int main() {
    const size_t ITERATIONS = 50000000;
    std::string strValue = "Hello";

    CallbackAndNoArgs<void(*)(size_t)> cb1NoArgs(exampleFunction1);
    // not possible: CallbackAndNoArgs<void(*)(size_t, std::string)> cb2NoArgs(exampleFunction2);

    CallbackAndArgsTupleCat<void(*)(size_t)> cb1TupleCat(exampleFunction1);
    CallbackAndArgsTupleCat<void(*)(size_t, std::string&), std::string> cb2TupleCat(exampleFunction2, strValue);

    CallbackAndArgsIndexSequence<void(*)(size_t)> cb1IndexSequence(exampleFunction1);
    CallbackAndArgsIndexSequence<void(*)(size_t, std::string&), std::string> cb2IndexSequence(exampleFunction2, strValue);
    


    // NoDynamicArgs(void):         ~ 163.3 Mio/sec
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoArgs.execute(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "NoDynamicArgs(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // TupleCat(void):              ~ 8.9 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1TupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "TupleCat(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;
    
    // TupleCat(std::string):       ~ 3.6 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2TupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "TupleCat(std::string): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // IndexSequence(void):         ~ 124.8 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1IndexSequence.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequence(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // IndexSequence(std::string):  ~ 43.2 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2IndexSequence.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequence(std::string): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    return 0;
}
