/**
 * Custom future/promise implementation that behaves like std::future but allows to
 * be moved and passed between threads.
 * 
 * @file Future.cpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#include "./Future.hpp"

using namespace spi;


Promise<void>::Promise(){
    this->state = new PromiseFutureState<void>();
    state->promiseRefs++;
}

Promise<void>::Promise(Promise<void> &&other){
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
}

Promise<void>& Promise<void>::operator=(Promise<void> &&other) {
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
    return *this;
}

Promise<void>::~Promise() noexcept(false) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->refs.fetch_sub(1) == 1){
        // this is last reference
        lockState.unlock();
        delete state;
        return;
    }
    if(state->promiseRefs.fetch_sub(1) == 1){
        if(!state->ready) // if this is last promise, state not ready, but still futures waiting
            throw std::runtime_error("Promise with waiting futures deleted before being fulfilled");
    }
}

bool Promise<void>::isFulfilled() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

void Promise<void>::set_value() noexcept(false) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready)
        throw std::runtime_error("Promise already fulfilled");
    state->ready = true;
    lockState.unlock();

    state->cvReady.notify_all();

    std::lock_guard<std::mutex> lockCallbacks(state->mCallbacks);
    for(auto &callback : state->callbacks)
        callback(state);
    state->callbacks.clear();
}

void Promise<void>::set_exception(const std::exception &exception) noexcept(false) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready)
        throw std::runtime_error("Promise already fulfilled");
    state->ready = true;
    state->exception = exception;
    lockState.unlock();

    state->cvReady.notify_all();

    std::lock_guard<std::mutex> lockCallbacks(state->mCallbacks);
    for(auto &callback : state->callbacks)
        callback(state);
    state->callbacks.clear();
}

Future<void> Promise<void>::get_future() {
    return Future<void>(state); // future will add itself to the state
}





Future<void>::Future(){
    this->state = new PromiseFutureState<void>();
    state->ready = true;
}

Future<void>::Future(const std::exception &exception){
    this->state = new PromiseFutureState<void>();
    state->ready = true;
    state->exception = exception;
}

Future<void>::Future(PromiseFutureState<void> *state) {
    this->state = state;
    state->refs++;
}

Future<void>::Future(const Future<void> &other){
    this->state = other.state;
    state->refs++;
}

Future<void>& Future<void>::operator=(const Future<void> &other){
    if(this != &other){
        this->state = other.state;
        state->refs++;
    }
    return *this;
}

Future<void>::Future(Future<void> &&other) {
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
}

Future<void>& Future<void>::operator=(Future<void> &&other){
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
    return *this;
}

Future<void>::~Future(){
    if(state->refs.fetch_sub(1) == 1){
        delete state; // no more references to this state
    }
}

bool Future<void>::is_ready() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

bool Future<void>::has_value() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready && !state->exception.has_value();
}

bool Future<void>::has_exception() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->exception.has_value();
}

void Future<void>::get_value() noexcept(false) {
    std::unique_lock<std::mutex> lock(state->mState);
    if(state->ready && !state->exception.has_value()){
        return;
    } else if(state->exception.has_value()){
        throw state->exception.value();
    } else {
        // wait for ready
        state->cvReady.wait(lock, [this]{ return state->ready; });

        if(state->ready && !state->exception.has_value())
            return;
        throw state->exception.value();
    }
}

std::exception Future<void>::get_exception() noexcept(false) {
    std::unique_lock<std::mutex> lock(state->mState);
    if(state->exception.has_value()) {
        return state->exception.value();
    } else if(state->ready) {
        throw std::runtime_error("Future has value instead of exception");
    } else {
        // wait for ready
        state->cvReady.wait(lock, [this]{ return state->ready; });
        
        if(state->exception.has_value())
            return state->exception.value();
        throw std::runtime_error("Future has value instead of exception");
    }
}


void Future<void>::onValue(std::function<void()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            callback();
            return;
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback();
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                callback();
            }
        });
    }
}

void Future<void>::onException(std::function<void(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            callback(state->exception.value());
            return;
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback(state->exception.value());
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                callback(state->exception.value());
            }
        });
    }
}


// callback taking void and returning R/void
template<typename R>
Future<R> Future<void>::then(std::function<R()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                if constexpr (std::is_same<R, void>::value) {
                    callback();
                    return Future<R>();
                } else {
                    return Future<R>(callback());
                }
            } catch (std::exception &exception) {
                return Future<R>(exception);
            }
        } else {
            return Future<R>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback();
                        return Future<R>();
                    } else {
                        return Future<R>(callback());
                    }
                } catch (std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback();
                        promise->set_value();
                    } else {
                        promise->set_value(callback());
                    }
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
            }
            delete promise;
        });
        return future;
    }
}

// callback taking void and returning shared_ptr<R>
template<typename R>
Future<std::shared_ptr<R>> Future<void>::then(std::function<std::shared_ptr<R>()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                return Future<std::shared_ptr<R>>(callback());
            } catch (std::exception &exception) {
                return Future<std::shared_ptr<R>>(exception);
            }
        } else {
            return Future<std::shared_ptr<R>>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return Future<std::shared_ptr<R>>(callback());
                } catch (std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    promise->set_value(callback());
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
            }
            delete promise;
        });
        return future;
    }
}

// callback taking void and returning Future<R>/Future<void>
template<typename R>
Future<R> Future<void>::thenFuture(std::function<Future<R>()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                return callback();
            } catch (std::exception &exception) {
                return Future<R>(exception);
            }
        } else {
            return Future<R>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback();
                } catch (std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    Future<R> future = callback();
                    if constexpr (std::is_same<R, void>::value) {
                        future.onValue([promise](){
                            promise->set_value();
                            delete promise;
                        });
                    } else {
                        future.onValue([promise](R value){
                            promise->set_value(value);
                            delete promise;
                        });
                    }
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
                delete promise;
            }
        });
        return future;
    }
}

// callback taking void and returning Future<shared_ptr<R>>
template<typename R>
Future<std::shared_ptr<R>> Future<void>::thenFuture(std::function<Future<std::shared_ptr<R>>()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                return callback();
            } catch (std::exception &exception) {
                return Future<std::shared_ptr<R>>(exception);
            }
        } else {
            return Future<std::shared_ptr<R>>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback();
                } catch (std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    Future<std::shared_ptr<R>> future = callback();
                    future.onValue([promise](std::shared_ptr<R> value){
                        promise->set_value(value);
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
                delete promise;
            }
        });
        return future;
    }
}


// callback taking exception and returning shared_ptr<T>
Future<void> Future<void>::catchAll(std::function<void(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            return Future<void>();
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                callback(exception);
                return Future<void>();
            } catch (std::exception &exception) {
                return Future<void>(exception);
            }
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                return Future<void>();
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    callback(exception);
                    return Future<void>();
                } catch (std::exception &exception) {
                    return Future<void>(exception);
                }
            }
        }

        // wait for this ready
        Promise<void> *promise = new Promise<void>();
        Future<void> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before setting value
                promise->set_value();
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    callback(exception);
                    promise->set_value();
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                }
            }
            delete promise;
        });
        return future;
    }
}

// callback taking exception and returning Future<shared_ptr<T>>
Future<void> Future<void>::catchAllFuture(std::function<Future<void>(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            return Future<void>();
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                return callback(exception);
            } catch (std::exception &exception) {
                return Future<void>(exception);
            }
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                return Future<void>();
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(exception);
                } catch (std::exception &exception) {
                    return Future<void>(exception);
                }
            }
        }

        // wait for this ready
        Promise<void> *promise = new Promise<void>();
        Future<void> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before setting value
                promise->set_value();
                delete promise;
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    Future<void> future = callback(exception);
                    future.onValue([promise](){
                        promise->set_value();
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            }
        });
        return future;
    }
}





// SPECIAL VOID IMPLEMENTATIONS FOR COMPILER TO BE ABLE TO PROPPERLY DEDUCE TEMPLATES

// callback taking void and returning void
template<>
Future<void> Future<void>::then(std::function<void()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                callback();
                return Future<void>();
            } catch (std::exception &exception) {
                return Future<void>(exception);
            }
        } else {
            return Future<void>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    callback();
                    return Future<void>();
                } catch (std::exception &exception) {
                    return Future<void>(exception);
                }
            } else {
                return Future<void>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<void> *promise = new Promise<void>();
        Future<void> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    callback();
                    promise->set_value();
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
            }
            delete promise;
        });
        return future;
    }
}

// callback taking void and returning Future<void>
template<>
Future<void> Future<void>::thenFuture(std::function<Future<void>()> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            lockState.unlock(); // unlock before callback
            try {
                return callback();
            } catch (std::exception &exception) {
                return Future<void>(exception);
            }
        } else {
            return Future<void>(state->exception.value());
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback();
                } catch (std::exception &exception) {
                    return Future<void>(exception);
                }
            } else {
                return Future<void>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<void> *promise = new Promise<void>();
        Future<void> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<void> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                lockState.unlock(); // unlock before callback
                try {
                    Future<void> future = callback();
                    future.onValue([promise](){
                        promise->set_value();
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before setting exception
                promise->set_exception(exception);
                delete promise;
            }
        });
        return future;
    }
}