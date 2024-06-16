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
    void execute(size_t mandatoryArg){
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
    void execute(size_t mandatoryArg){
        auto fullArgs = std::tuple_cat(std::make_tuple(mandatoryArg), args);
        std::apply(callback, fullArgs);
    }
};




// std::index_sequence with helper (WINNER)
template<typename Callback, typename... Args>
struct CallbackAndArgsIndexSequenceWithHelper {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackAndArgsIndexSequenceWithHelper(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Helper function to invoke the callback with the mandatory size_t and additional args
    template <std::size_t... I>
    void invoke(std::index_sequence<I...>, size_t mandatoryArg){
        callback(mandatoryArg, std::get<I>(args)...);
    }

    // Execute the stored callback with its arguments, including the mandatory size_t
    void execute(size_t mandatoryArg){
        invoke(std::index_sequence_for<Args...>{}, mandatoryArg);
    }
};



// std::index_sequence with lambda
template<typename Callback, typename... Args>
struct CallbackAndArgsIndexSequenceNoHelper {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackAndArgsIndexSequenceNoHelper(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    void execute(size_t mandatoryArg){
        std::apply([this, mandatoryArg](auto... indices) {
            callback(mandatoryArg, indices...);
        }, args);
    }
};



// Example functions to be used as callbacks
void exampleFunction1(size_t a) {
    size_t c = a + a;
    (void)c;
}

// Example functions to be used as callbacks
void exampleFunction2(size_t a, size_t b) {
    size_t c = a + b;
    (void)c;
}




int main() {
    const size_t ITERATIONS = 50000000;
    size_t additionalValue = 42; // can be of any type

    CallbackAndNoArgs<void(*)(size_t)> cb1NoArgs(exampleFunction1);
    // not possible: CallbackAndNoArgs<void(*)(size_t, size_t)> cb2NoArgs(exampleFunction2);

    CallbackAndArgsTupleCat<void(*)(size_t)> cb1TupleCat(exampleFunction1);
    CallbackAndArgsTupleCat<void(*)(size_t, size_t), size_t> cb2TupleCat(exampleFunction2, additionalValue);

    CallbackAndArgsIndexSequenceWithHelper<void(*)(size_t)> cb1IndexSequenceWithHelper(exampleFunction1);
    CallbackAndArgsIndexSequenceWithHelper<void(*)(size_t, size_t), size_t> cb2IndexSequenceWithHelper(exampleFunction2, additionalValue);

    CallbackAndArgsIndexSequenceNoHelper<void(*)(size_t)> cb1IndexSequenceNoHelper(exampleFunction1);
    CallbackAndArgsIndexSequenceNoHelper<void(*)(size_t, size_t), size_t> cb2IndexSequenceNoHelper(exampleFunction2, additionalValue);
    


    // NoDynamicArgs(void):                 ~ 168.0 Mio/sec
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoArgs.execute(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "NoDynamicArgs(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // TupleCat(void):                          ~ 9.1 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1TupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "TupleCat(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;
    
    // TupleCat(std::string):                   ~ 6.8 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2TupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "TupleCat(std::string): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // IndexSequenceWithHelper(void):           ~ 126.6 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1IndexSequenceWithHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequenceWithHelper(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // IndexSequenceWithHelper(std::string):    ~ 49.2 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2IndexSequenceWithHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequenceWithHelper(std::string): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // IndexSequenceNoHelper(void):           ~ 33.6 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1IndexSequenceNoHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequenceNoHelper(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // IndexSequenceNoHelper(std::string):    ~ 22.6 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2IndexSequenceNoHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "IndexSequenceNoHelper(std::string): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;


    return 0;
}
