#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>



int funcPointerI = 0, tempParamI = 0, funcObjI = 0;


void functionPointer(void (*callback)(int&)){
    if(callback) callback(funcPointerI);
}


template<typename Callback>
void templateParameter(Callback callback){
    callback(tempParamI);
}
template<>
void templateParameter(std::nullptr_t){
    tempParamI++;
}


void functionObject(std::function<void(int&)> callback){
    if(callback) callback(funcObjI);
}





void doSomething(int &a){
    a++;
}

struct StructFunctor {
    void operator()(int& a) const {
        a++;
    }
};



int main(){
    const uint64_t ITERATIONS = 100000000;

    StructFunctor structFunctor;


    // CONCLUSION:
    //
    // - IMPORTANT:  for function definition use templates e.g.
    //      template<typename Callback>
    //      void function(Callback callback)
    //
    // - NEGLIGIBLE: passed callbacks are ideally funtors (structs with operator()) or functions
    //



    // FunctionPointer(Function):           ~ 168.3 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer(doSomething);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointer(Lambda):             ~ 82.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer([](int& a){a++;});
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointer(StructFunctor):      ---
    /* NOT POSSIBLE
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer(structFunctor);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(StructFunctor): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;*/
    std::cout << "FunctionPointer(StructFunctor):  --- " << std::endl;

    // FunctionPointer(nullptr):             ~ 431.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;




    // TemplateParameter(Function):         ~ 134.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter(doSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // TemplateParameter(Lambda):           ~ 139.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter([](int& a){a++;});
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // TemplateParameter(StructFunctor):    ~ 164.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter(structFunctor);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(StructFunctor): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // TemplateParameter(nullptr):          ~ 479.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;




    // std::function(Function):             ~ 11.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject(doSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::function(Lambda):               ~ 10.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject([](int& a){a++;});
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::function(StructFuntor):         ~ 11.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject(structFunctor);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(StructFuntor): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::function(nullptr):              ~ 50.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;



    return 0;
}