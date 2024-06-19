#include <iostream>
#include <functional>
#include <tuple>
#include <string>
#include <chrono>


// [ REFERENCE ]
// no dynamic args
template<typename Callback>
struct CallbackAndNoArgs {
    Callback callback;

    // Constructor
    CallbackAndNoArgs(Callback cb)
        : callback(cb) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(size_t mandatoryArg){
        callback(mandatoryArg);
    }
};




// [ NO MANDATORY ARGS ]
// std::apply
template<typename Callback, typename... Args>
struct CallbackNoMandatoryApply {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackNoMandatoryApply(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(){
        std::apply(callback, args);
    }
};


// [ NO MANDATORY ARGS ]
// std::index_sequence with helper
template<typename Callback, typename... Args>
struct CallbackNoMandatoryIndexSeqHelper {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackNoMandatoryIndexSeqHelper(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Helper function to invoke the callback with the mandatory size_t and additional args
    template <std::size_t... I>
    inline void invoke(std::index_sequence<I...>){
        callback(std::get<I>(args)...);
    }

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(){
        invoke(std::index_sequence_for<Args...>{});
    }
};


// [ NO MANDATORY ARGS ]
// std::index_sequence with lambda
template<typename Callback, typename... Args>
struct CallbackNoMandatoryIndexSeqLambda {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackNoMandatoryIndexSeqLambda(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(){
        std::apply([this](auto... indices) {
            callback(indices...);
        }, args);
    }
};




// [ WITH MANDATORY ARGS ]
// std::tuple_cat
template<typename Callback, typename... Args>
struct CallbackWithMandatoryTupleCat {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackWithMandatoryTupleCat(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(size_t mandatoryArg){
        auto fullArgs = std::tuple_cat(std::make_tuple(mandatoryArg), args);
        std::apply(callback, fullArgs);
    }
};


// [ WITH MANDATORY ARGS ]  <-- WINNER
// std::index_sequence with helper
template<typename Callback, typename... Args>
struct CallbackWithMandatoryIndexSeqHelper {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackWithMandatoryIndexSeqHelper(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Helper function to invoke the callback with the mandatory size_t and additional args
    template <std::size_t... I>
    inline void invoke(std::index_sequence<I...>, size_t mandatoryArg){
        callback(mandatoryArg, std::get<I>(args)...);
    }

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(size_t mandatoryArg){
        invoke(std::index_sequence_for<Args...>{}, mandatoryArg);
    }
};


// [ WITH MANDATORY ARGS ]
// std::index_sequence with lambda
template<typename Callback, typename... Args>
struct CallbackWithMandatoryIndexSeqLambda {
    Callback callback;
    std::tuple<Args...> args;

    // Constructor
    CallbackWithMandatoryIndexSeqLambda(Callback cb, Args... arguments)
        : callback(cb), args(std::make_tuple(arguments...)) {}

    // Execute the stored callback with its arguments, including the mandatory size_t
    inline void execute(size_t mandatoryArg){
        std::apply([this, mandatoryArg](auto... indices) {
            callback(mandatoryArg, indices...);
        }, args);
    }
};






// Example functions to be used as callbacks (only used by no mandatory to keep dynamic args count equal to with mandatory)
void exampleFunction0() {
    volatile size_t a = 42;
    size_t c = a + a;
    (void)c;
}


// Example functions to be used as callbacks
void exampleFunction1(size_t a) {
    volatile size_t c = a + a;
    (void)c;
}

// Example functions to be used as callbacks
void exampleFunction2(size_t a, size_t b) {
    volatile size_t c = a + b;
    (void)c;
}




int main() {
    const size_t ITERATIONS = 50000000;
    size_t additionalValue = 42; // can be of any type



    /*
        !---------------------------------------------!

        CONCLUSION: Always choose std::index_sequence with helper 
                    regardless of mandatory args or not.

        !---------------------------------------------!
    */



    // REFERENCE
    CallbackAndNoArgs<void(*)(size_t)> cb1NoArgs(exampleFunction1);
    // not possible: CallbackAndNoArgs<void(*)(size_t, size_t)> cb2NoArgs(exampleFunction2);


    // NO MANDATORY ARGS
    CallbackNoMandatoryApply<void(*)()> cb1NoMandatoryApply(exampleFunction0);
    CallbackNoMandatoryApply<void(*)(size_t), size_t> cb2NoMandatoryApply(exampleFunction1, additionalValue);

    CallbackNoMandatoryIndexSeqHelper<void(*)()> cb1NoMandatoryIndexSeqHelper(exampleFunction0);
    CallbackNoMandatoryIndexSeqHelper<void(*)(size_t), size_t> cb2NoMandatoryIndexSeqHelper(exampleFunction1, additionalValue);

    CallbackNoMandatoryIndexSeqLambda<void(*)()> cb1NoMandatoryIndexSeqLambda(exampleFunction0);
    CallbackNoMandatoryIndexSeqLambda<void(*)(size_t), size_t> cb2NoMandatoryIndexSeqLambda(exampleFunction1, additionalValue);


    // WITH MANDATORY ARGS
    CallbackWithMandatoryTupleCat<void(*)(size_t)> cb1WithMandatoryTupleCat(exampleFunction1);
    CallbackWithMandatoryTupleCat<void(*)(size_t, size_t), size_t> cb2WithMandatoryTupleCat(exampleFunction2, additionalValue);

    CallbackWithMandatoryIndexSeqHelper<void(*)(size_t)> cb1WithMandatoryIndexSeqHelper(exampleFunction1);
    CallbackWithMandatoryIndexSeqHelper<void(*)(size_t, size_t), size_t> cb2WithMandatoryIndexSeqHelper(exampleFunction2, additionalValue);

    CallbackWithMandatoryIndexSeqLambda<void(*)(size_t)> cb1WithMandatoryIndexSeqLambda(exampleFunction1);
    CallbackWithMandatoryIndexSeqLambda<void(*)(size_t, size_t), size_t> cb2WithMandatoryIndexSeqLambda(exampleFunction2, additionalValue);
    

    //                                                      RELEASE         |   DEBUG


    // [ REFERENCE ]

    // NoDynamicArgs(void):                                 ~ 1960 Mio/sec  |   ~ 171.3 Mio/sec
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoArgs.execute(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "NoDynamicArgs(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;
    std::cout << std::endl;





    // [ NO MANDATORY ARGS ]

    // CallbackNoMandatoryApply(void):                      ~ 1142 Mio/sec  |   ~ 38.0 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoMandatoryApply.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryArgs(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackNoMandatoryApply(size_t):                    ~ 1559 Mio/sec  |   ~ 24.1 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2NoMandatoryApply.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryArgs(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // CallbackNoMandatoryIndexSeqHelper(void):             ~ 1029 Mio/sec  |   ~ 137.1 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoMandatoryIndexSeqHelper.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryIndexSeqHelper(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackNoMandatoryIndexSeqHelper(size_t):           ~ 2920 Mio/sec  |   ~ 53.2 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2NoMandatoryIndexSeqHelper.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryIndexSeqHelper(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // CallbackNoMandatoryIndexSeqLambda(void):             ~ 2656 Mio/sec  |   ~ 33.1 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1NoMandatoryIndexSeqLambda.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryIndexSeqLambda(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackNoMandatoryIndexSeqLambda(size_t):           ~ 1555 Mio/sec  |   ~ 22.8 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2NoMandatoryIndexSeqLambda.execute();
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "NoMandatoryIndexSeqLambda(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;
    std::cout << std::endl;





    // [ WITH MANDATORY ARGS ]

    // CallbackWithMandatoryTupleCat(void):                 ~ 1576 Mio/sec  |   ~ 8.8 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1WithMandatoryTupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryTupleCat(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackWithMandatoryTupleCat(size_t):               ~ 2763 Mio/sec  |   ~ 6.7 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2WithMandatoryTupleCat.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryTupleCat(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // CallbackWithMandatoryIndexSeqHelper(void):           ~ 2801 Mio/sec  |   ~ 122.5 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1WithMandatoryIndexSeqHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryIndexSeqHelper(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackWithMandatoryIndexSeqHelper(size_t):         ~ 2800 Mio/sec  |   ~ 49.8 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2WithMandatoryIndexSeqHelper.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryIndexSeqHelper(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;



    // CallbackWithMandatoryIndexSeqLambda(void):           ~ 1551 Mio/sec  |   ~ 31.5 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb1WithMandatoryIndexSeqLambda.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryIndexSeqLambda(void): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;

    // CallbackWithMandatoryIndexSeqLambda(size_t):         ~ 1544 Mio/sec  |   ~ 22.4 Mio/sec
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ITERATIONS; ++i) {
        cb2WithMandatoryIndexSeqLambda.execute(i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "WithMandatoryIndexSeqLambda(size_t): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "/s" << std::endl;
    std::cout << std::endl;

    return 0;
}
