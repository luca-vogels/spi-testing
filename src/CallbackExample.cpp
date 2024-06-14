#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>


class NaiveBudget {
public:

    void get(std::function<void()> doneCb){
        doneCb();
    }
};

class NaiveBase {
public:

    virtual void doSomething(std::function<void()> doneCb) = 0;
};

class NaiveDerived : public NaiveBase {
private:
    NaiveBudget budget;
public:

    void doSomething(std::function<void()> doneCb) override {
        budget.get([&doneCb](){
            doneCb();
        });
    }
};

class NaiveExecutor {
private:
    NaiveDerived derived;
    size_t counter = 0;
public:

    void execute(){
        derived.doSomething([this](){
            this->counter++;
        });
    }
};



// ------------------------------



template<typename DoneCallback, typename... Args>
class NewBudget {
public:
    
    void get(DoneCallback doneCb, Args... args){
        doneCb(args...);
    }
};

template<typename DoneCallback, typename... Args>
class NewBase {

public:

    virtual void doSomething(DoneCallback doneCb, Args... args) = 0;
};

template<typename DoneCallback, typename... Args>
class NewDerived : public NewBase<DoneCallback, Args...> {
private:

    NewBudget<void(*)(NewDerived *obj, DoneCallback doneCb, Args... args), NewDerived*, DoneCallback, Args...> budget;
public:

    static void _continue(NewDerived *obj, DoneCallback doneCb, Args... args){
        (void)obj;
        doneCb(args...);
    }
    
    void doSomething(DoneCallback doneCb, Args... args) override {
        budget.get(NewDerived::_continue, this, doneCb, args...);
    }
};

class NewExecutor {
private:
    NewDerived<void(*)(NewExecutor *obj), NewExecutor*> derived;
public:
    size_t _counter = 0;

    static void _continue(NewExecutor *obj){
        obj->_counter++;
    }

    void execute(){
        derived.doSomething(NewExecutor::_continue, this);
    }
};

// ------------------------------





int main(){
    const uint64_t ITERATIONS = 50000000;

    NaiveExecutor naiveExecutor;
    NewExecutor newExecutor;


    

    // NaiveExecutor:   ~ 6.0 Mio/sec
    auto startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        naiveExecutor.execute();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::cout << "NaiveExecutor: " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    // NewExecutor:     ~ 59.5 Mio/sec
    startTime = std::chrono::high_resolution_clock::now();
    for(uint64_t i=0; i < ITERATIONS; i++){
        newExecutor.execute();
    }
    endTime = std::chrono::high_resolution_clock::now();
    std::cout << "NewExecutor:  " << (ITERATIONS * 1000000) / std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() << "/s" << std::endl;


    return 0;
}