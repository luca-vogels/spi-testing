#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>


// FUNCTIONAL PROGRAMMING

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




// OBJECT ORIENTED PROGRAMMING

class Base {
protected:
    int a = 0;
public:

    void doOnBase(){
        this->a++;
    }

    virtual void doOnVirtual() = 0;



    virtual void functionPointer(void (*callback)(int&)) = 0;

    /* VIRTUAL METHODS CANNOT USE TEMPLATES
    template<typename Callback>
    virtual void templateParameter(Callback callback) = 0; */

    virtual void functionObject(std::function<void(int&)> callback) = 0;

    virtual ~Base() = default;
};

class Derived : public Base {
public:

    void doOnVirtual() override {
        this->a++;
    }


    void functionPointer(void (*callback)(int&)) override {
        if(callback) callback(this->a);
    }

    /* VIRTUAL METHODS CANNOT USE TEMPLATES
    template<typename Callback>
    void templateParameter(Callback callback) override {
        callback(tempParamI);
    }*/

    void functionObject(std::function<void(int&)> callback) override {
        if(callback) callback(this->a);
    }
 
};




// MIXED PROGRAMMING

void functionPointerOnObject(void (*callback)(Base*), Base* obj){
    if(callback) callback(obj);
}

template<typename Callback>
void templateParameterOnObject(Callback callback, Base* obj){
    callback(obj);
}
template<>
void templateParameterOnObject(std::nullptr_t, Base* obj){
    (void)obj;
}

void functionObjectOnObject(std::function<void(Base*)> callback, Base* obj){
    if(callback) callback(obj);
}



void doOnBase(Base* obj){
    obj->doOnBase();
}

void doOnVirtual(Base* obj){
    obj->doOnVirtual();
}






int main(){
    const uint64_t ITERATIONS = 100000000;

    StructFunctor structFunctor;
    Base* obj = new Derived();

    auto objDoSomething = std::bind(&Base::doOnBase, obj);



    // IMPORTANT TO KNOW:
    //
    //  - function pointers (void (*callback)(int&)) do accept lambda functions
    //    but the lambdas cannot captured any variables!
    //    e.g. [this](){return a;} is not allowed
    //
    //
    // CONCLUSION:
    //
    // - NEVER USE std::bind() !!!
    //
    // - PREFER Functional Programming (FP) over Object Oriented Programming (OOP)
    //      - in object oriented programming you can create performance critical
    //        code inside functions and simply pass the object as argument
    //        to get a mix of both worlds.
    //
    // - PREFER Templates for callback over std::function (not possible for virtual methods)
    //
    // - NORMAL FUNCTIONS AND METHODS
    //      - IMPORTANT:  for function definition use templates e.g.
    //          template<typename Callback>
    //          void foo(Callback callback)
    //
    //          template<>
    //          void foo(std::nullptr_t)
    //
    //      - NEGLIGIBLE: passed callbacks are ideally funtors (structs with operator()) or functions
    //
    // 
    // - VIRTUAL METHODS
    //      - IMPORTANT:  use function pointers (void (*callback)(int&)) over std::function!




    std::cout << "FUNCTIONAL PROGRAMMING:" << std::endl;

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


    // FunctionPointer(ObjMethodBind):      ---
    /* NOT POSSIBLE
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer(objDoSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(ObjMethodBind): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;*/
    std::cout << "FunctionPointer(ObjMethodBind):  --- " << std::endl;


    // FunctionPointer(nullptr):             ~ 431.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointer(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointer(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





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


    // TemplateParameter(ObjMethodBind):    ~ 16.0 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter(objDoSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(ObjMethodBind): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // TemplateParameter(nullptr):          ~ 479.1 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        templateParameter(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "TemplateParameter(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





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


    // std::function(ObjMethodBind):        ~ 6.4 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject(objDoSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(ObjMethodBind): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // std::function(nullptr):              ~ 58.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionObject(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "std::function(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;



    

    std::cout << "OBJECT ORIENTED PROGRAMMING:" << std::endl;

    // obj->functionPointer(Function):      ~ 93.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionPointer(doSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionPointer(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // obj->functionPointer(Lambda):        ~ 84.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionPointer([](int& a){a++;});
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionPointer(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // obj->functionPointer(StructFunctor): ---
    /* NOT POSSIBLE
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionPointer(structFunctor);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionPointer(StructFunctor): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;*/
    std::cout << "obj->functionPointer(StructFunctor):  --- " << std::endl;


    // obj->functionPointer(ObjMethodBind): ---
    /* NOT POSSIBLE
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionPointer(objDoSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionPointer(ObjMethodBind): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;*/
    std::cout << "obj->functionPointer(ObjMethodBind):  --- " << std::endl;


    // obj->functionPointer(nullptr):       ~ 413.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionPointer(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionPointer(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;



    // VIRTUAL METHODS CANNOT USE TEMPLATES
    std::cout << "Virtual methods cannot use templated" << std::endl;
    std::cout << std::endl;



    // obj->functionObject(Function):       ~ 10.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionObject(doSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionObject(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    

    // obj->functionObject(Lambda):         ~ 10.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionObject([](int& a){a++;});
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionObject(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // obj->functionObject(StructFunctor):  ~ 10.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionObject(structFunctor);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionObject(StructFunctor): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // obj->functionObject(ObjMethodBind):  ~ 5.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionObject(objDoSomething);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionObject(ObjMethodBind): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // obj->functionObject(nullptr):        ~ 55.8 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        obj->functionObject(nullptr);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "obj->functionObject(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    std::cout << "MIXED PROGRAMMING:" << std::endl;

    // FunctionPointerOnBase(Function):     ~ 104.9 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject(doOnBase, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnBase(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointerOnBase(Lambda):       ~ 75.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject([](Base* obj){ obj->doOnBase(); }, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnBase(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointerOnBase(nullptr):      ~ 450.6 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject(nullptr, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnBase(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;





    // FunctionPointerOnVirtual(Function):  ~ 68.7 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject(doOnVirtual, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnVirtual(Function): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointerOnVirtual(Lambda):    ~ 55.3 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject([](Base* obj){ obj->doOnVirtual(); }, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnVirtual(Lambda): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // FunctionPointerOnVirtual(nullptr):   ~ 471.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        functionPointerOnObject(nullptr, obj);
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "FunctionPointerOnVirtual(nullptr): " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;
    std::cout << std::endl;


    delete obj;
    return 0;
}