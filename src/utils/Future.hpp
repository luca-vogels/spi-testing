/**
 * Custom future/promise implementation that behaves like std::future but allows to
 * be moved and passed between threads.
 * 
 * @file Future.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_FUTURE_HPP
#define SPI_FUTURE_HPP

#include <atomic>
#include <condition_variable>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <type_traits> // if constexpr (std::is_same_v<R, void>)
#include <vector>

namespace spi {

template<typename T>
struct PromiseFutureState; // deferred declaration (see below)

template<typename T>
struct PromiseFutureState<std::shared_ptr<T>>; // deferred declaration (see below)

template<>
struct PromiseFutureState<void>; // deferred declaration (see below)


template <typename T>
class Future; // deferred declaration (see below)

template <typename T>
class Future<std::shared_ptr<T>>; // deferred declaration (see below)

template <>
class Future<void>; // deferred declaration (see below)




// ---------------------------------------------------------------------
// ----- [ PROMISE DECLARATION ] ---------------------------------------
// ---------------------------------------------------------------------


/**
 * A Promise represents a future value that is not yet known.
 * The actual value is presented by the Future object that this Promise returns.
 * The Future can be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template <typename T>
class Promise {
protected:

    PromiseFutureState<T> *state; // deleted by last future or promise pointing to it

public:

    /**
     * Creates a new promise that will be fulfilled in the future.
     * Returns a future value that can be set later through this promise.
     */
    Promise();


    Promise(const Promise<T> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise& operator=(const Promise<T> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise(Promise<T> &&other);
    Promise& operator=(Promise<T> &&other);

    /**
     * Destructor
     * @throws Throws a runtime_exception if promise has not been fulfilled yet but there are still futures waiting for it.
     */
    ~Promise() noexcept(false);


    /**
     * Returns if this promise has been fulfilled (either value or exception has been set).
     * If a promise is fulfilled, all previously created futures will be ready and 
     * all new futures will be set to the value or exception immediately.
     * @return true if this promise has been fulfilled, false otherwise.
     */
    bool isFulfilled() const;

    /**
     * Sets the value and updates all futures belonging by this promise.
     * 
     * @param value Value that should be set.
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_value(T value) noexcept(false);

    /**
     * Sets the exception and updates all futures belonging to this promise.
     * 
     * @param exception Exception that should be set.
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_exception(const std::exception &exception) noexcept(false);

    /**
     * Returns a new future that will be set to the value or exception 
     * as soon as it is set for this promise.
     * If promise has already been fulfilled, the future will be set to the value or exception immediately.
     * 
     * @return Future<T> Future representing the value or exception that will be available.
     */
    Future<T> get_future();
};



/**
 * A Promise represents a future value that is not yet known.
 * The actual value is presented by the Future object that this Promise returns.
 * The Future can be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template <typename T>
class Promise<std::shared_ptr<T>> {
protected:

    PromiseFutureState<std::shared_ptr<T>> *state; // deleted by last future or promise pointing to it

public:

    /**
     * Creates a new promise that will be fulfilled in the future.
     * Returns a future value that can be set later through this promise.
     */
    Promise();


    Promise(const Promise<std::shared_ptr<T>> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise& operator=(const Promise<std::shared_ptr<T>> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise(Promise<std::shared_ptr<T>> &&other);
    Promise& operator=(Promise<std::shared_ptr<T>> &&other);

    /**
     * Destructor
     * @throws Throws a runtime_exception if promise has not been fulfilled yet but there are still futures waiting for it.
     */
    ~Promise() noexcept(false);


    /**
     * Returns if this promise has been fulfilled (either value or exception has been set).
     * If a promise is fulfilled, all previously created futures will be ready and 
     * all new futures will be set to the value or exception immediately.
     * @return true if this promise has been fulfilled, false otherwise.
     */
    bool isFulfilled() const;

    /**
     * Sets the value and updates all futures belonging by this promise.
     * 
     * @param value Value that should be set.
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_value(std::shared_ptr<T> value) noexcept(false);

    /**
     * Sets the exception and updates all futures belonging to this promise.
     * 
     * @param exception Exception that should be set.
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_exception(const std::exception &exception) noexcept(false);

    /**
     * Returns a new future that will be set to the value or exception 
     * as soon as it is set for this promise.
     * If promise has already been fulfilled, the future will be set to the value or exception immediately.
     * 
     * @return Future<std::shared_ptr<T>> Future representing the value or exception that will be available.
     */
    Future<std::shared_ptr<T>> get_future();
};



/**
 * A Promise represents a future value that is not yet known.
 * The actual value is presented by the Future object that this Promise returns.
 * The Future can be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template <>
class Promise<void> {
protected:

    PromiseFutureState<void> *state; // deleted by last future or promise pointing to it

public:

    /**
     * Creates a new promise that will be fulfilled in the future.
     * Returns a future value that can be set later through this promise.
     */
    Promise();


    Promise(const Promise<void> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise& operator=(const Promise<void> &other) = delete; // copying not allowed otherwise two promises that want to fulfill same future!
    Promise(Promise<void> &&other);
    Promise& operator=(Promise<void> &&other);

    /**
     * Destructor
     * @throws Throws a runtime_exception if promise has not been fulfilled yet but there are still futures waiting for it.
     */
    ~Promise() noexcept(false);


    /**
     * Returns if this promise has been fulfilled (either value or exception has been set).
     * If a promise is fulfilled, all previously created futures will be ready and 
     * all new futures will be set to the value or exception immediately.
     * @return true if this promise has been fulfilled, false otherwise.
     */
    bool isFulfilled() const;

    /**
     * Sets the value and updates all futures belonging by this promise.
     * 
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_value() noexcept(false);

    /**
     * Sets the exception and updates all futures belonging to this promise.
     * 
     * @param exception Exception that should be set.
     * @throws Throws a runtime_exception if promise has already been resolved.
     */
    void set_exception(const std::exception &exception) noexcept(false);

    /**
     * Returns a new future that will be set to the value or exception 
     * as soon as it is set for this promise.
     * If promise has already been fulfilled, the future will be set to the value or exception immediately.
     * 
     * @return Future<void> Future representing the value or exception that will be available.
     */
    Future<void> get_future();
};





// ---------------------------------------------------------------------
// ----- [ FUTURE DECLARATION ] ----------------------------------------
// ---------------------------------------------------------------------


/** 
 * A Future represents a value that will be available in the future.
 * It is returned by a Promise and can only be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template<typename T>
class Future {
    friend class Promise<T>; // so promise can access protected constructor of this future
protected:

    PromiseFutureState<T> *state; // deleted by last future or promise pointing to it

    Future(PromiseFutureState<T> *state);

public:

    /** Creates a new Future that will be set to the given value.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     * 
     * @param value Value that the Future will be set to.
     */
    Future(T value);

    /** Creates a new Future that will be set to the given exception.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     * 
     * @param exception Exception that the Future will be set to.
     */
    Future(const std::exception &exception);


    Future() = delete;
    Future(const Future<T> &other);
    Future& operator=(const Future<T> &other);
    Future(Future<T> &&other);
    Future& operator=(Future<T> &&other);
    ~Future();


    /**
     * Returns if the Future has retrieved a result yet.
     * 
     * @return true if the Future has retrieved a value or a result, false otherwise.
     */
    bool is_ready() const;

    /**
     * Returns if the future has retrieved a value.
     * 
     * @return true if value is available, false otherwise.
     */
    bool has_value() const;

    /**
     * Returns if the future has retrieved an exception.
     * 
     * @return true if exception is available, false otherwise.
     */
    bool has_exception() const;

    /**
     * Returns the value that has been retrieved.
     * If value is not available, this will block until it is.
     * If an exception has been retrieved instead of a value, this will throw the exception.
     * 
     * @return T Value that has been retrieved.
     * @throws Throws an exception if an exception has been retrieved instead of a value.
     */
    T get_value() noexcept(false);

    /**
     * Returns the exception that has been retrieved.
     * If exception is not available, this will block until it is.
     * If a value has been retrieved instead of an exception, this will throw 
     * 
     * @return std::exception& Exception that has been retrieved.
     * @throws Throws a runtime_error if a value has been retrieved instead of an exception.
     */
    std::exception get_exception() noexcept(false);


    /**
     * Invokes the callback function as soon as the value of this Future is available.
     * If the value is already available, the callback function will be executed immediately.
     * If an exception is available instead of a value, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     */
    void onValue(std::function<void(T)> callback) noexcept(true);

    /**
     * Invokes the callback function as soon as the exception of this Future is available.
     * If the exception is already available, the callback function will be executed immediately.
     * If a value is available instead of an exception, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the exception of this Future is available.
     */
    void onException(std::function<void(std::exception)> callback) noexcept(true);


    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> then(std::function<R(T)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> then(std::function<std::shared_ptr<R>(T)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> thenFuture(std::function<Future<R>(T)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> thenFuture(std::function<Future<std::shared_ptr<R>>(T)> callback) noexcept(true);


    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<T> catchAll(std::function<T(std::exception)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<T> catchAllFuture(std::function<Future<T>(std::exception)> callback) noexcept(true);

};



/** 
 * A Future represents a value that will be available in the future.
 * It is returned by a Promise and can only be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template<typename T>
class Future<std::shared_ptr<T>> {
    friend class Promise<std::shared_ptr<T>>; // so promise can access protected constructor of this future
protected:

    PromiseFutureState<std::shared_ptr<T>> *state; // deleted by last future or promise pointing to it

    Future(PromiseFutureState<std::shared_ptr<T>> *state);

public:

    /** Creates a new Future that will be set to the given value.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     * 
     * @param value Value that the Future will be set to.
     */
    Future(std::shared_ptr<T> value);

    /** Creates a new Future that will be set to the given exception.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     * 
     * @param exception Exception that the Future will be set to.
     */
    Future(const std::exception &exception);


    Future() = delete;
    Future(const Future<std::shared_ptr<T>> &other);
    Future& operator=(const Future<std::shared_ptr<T>> &other);
    Future(Future<std::shared_ptr<T>> &&other);
    Future& operator=(Future<std::shared_ptr<T>> &&other);
    ~Future();


    /**
     * Returns if the Future has retrieved a result yet.
     * 
     * @return true if the Future has retrieved a value or a result, false otherwise.
     */
    bool is_ready() const;

    /**
     * Returns if the future has retrieved a value.
     * 
     * @return true if value is available, false otherwise.
     */
    bool has_value() const;

    /**
     * Returns if the future has retrieved an exception.
     * 
     * @return true if exception is available, false otherwise.
     */
    bool has_exception() const;

    /**
     * Returns the value that has been retrieved.
     * If value is not available, this will block until it is.
     * If an exception has been retrieved instead of a value, this will throw the exception.
     * 
     * @return T Value that has been retrieved.
     * @throws Throws an exception if an exception has been retrieved instead of a value.
     */
    std::shared_ptr<T> get_value() noexcept(false);

    /**
     * Returns the exception that has been retrieved.
     * If exception is not available, this will block until it is.
     * If a value has been retrieved instead of an exception, this will throw 
     * 
     * @return std::exception& Exception that has been retrieved.
     * @throws Throws a runtime_error if a value has been retrieved instead of an exception.
     */
    std::exception get_exception() noexcept(false);


    /**
     * Invokes the callback function as soon as the value of this Future is available.
     * If the value is already available, the callback function will be executed immediately.
     * If an exception is available instead of a value, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     */
    void onValue(std::function<void(std::shared_ptr<T>)> callback) noexcept(true);

    /**
     * Invokes the callback function as soon as the exception of this Future is available.
     * If the exception is already available, the callback function will be executed immediately.
     * If a value is available instead of an exception, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the exception of this Future is available.
     */
    void onException(std::function<void(std::exception)> callback) noexcept(true);

    
    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> then(std::function<R(std::shared_ptr<T>)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> then(std::function<std::shared_ptr<R>(std::shared_ptr<T>)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> thenFuture(std::function<Future<R>(std::shared_ptr<T>)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> thenFuture(std::function<Future<std::shared_ptr<R>>(std::shared_ptr<T>)> callback) noexcept(true);


    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<std::shared_ptr<T>> catchAll(std::function<std::shared_ptr<T>(std::exception)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<std::shared_ptr<T>> catchAllFuture(std::function<Future<std::shared_ptr<T>>(std::exception)> callback) noexcept(true);
};



/** 
 * A Future represents a value that will be available in the future.
 * It is returned by a Promise and can only be modified by the Promise.
 * 
 * @tparam T Type of the future value.
 */
template <>
class Future<void> {
    friend class Promise<void>; // so promise can access protected constructor of this future
protected:

    PromiseFutureState<void> *state; // deleted by last future or promise pointing to it

    Future(PromiseFutureState<void> *state);

public:

    /** Creates a new Future that will be set to the given value.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     */
    Future();

    /** Creates a new Future that will be set to the given exception.
     * IMPORTANT: this will only get automatically deleted when returned from within a then<> or catchAll<> callback!
     * 
     * @param exception Exception that the Future will be set to.
     */
    Future(const std::exception &exception);


    Future(const Future<void> &other);
    Future& operator=(const Future<void> &other);
    Future(Future<void> &&other);
    Future& operator=(Future<void> &&other);
    ~Future();


    /**
     * Returns if the Future has retrieved a result yet.
     * 
     * @return true if the Future has retrieved a value or a result, false otherwise.
     */
    bool is_ready() const;

    /**
     * Returns if the future has retrieved a value.
     * 
     * @return true if value is available, false otherwise.
     */
    bool has_value() const;

    /**
     * Returns if the future has retrieved an exception.
     * 
     * @return true if exception is available, false otherwise.
     */
    bool has_exception() const;

    /**
     * Returns the value that has been retrieved.
     * If value is not available, this will block until it is.
     * If an exception has been retrieved instead of a value, this will throw the exception.
     * 
     * @return T Value that has been retrieved.
     * @throws Throws an exception if an exception has been retrieved instead of a value.
     */
    void get_value() noexcept(false);

    /**
     * Returns the exception that has been retrieved.
     * If exception is not available, this will block until it is.
     * If a value has been retrieved instead of an exception, this will throw 
     * 
     * @return std::exception& Exception that has been retrieved.
     * @throws Throws a runtime_error if a value has been retrieved instead of an exception.
     */
    std::exception get_exception() noexcept(false);


    /**
     * Invokes the callback function as soon as the value of this Future is available.
     * If the value is already available, the callback function will be executed immediately.
     * If an exception is available instead of a value, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     */
    void onValue(std::function<void()> callback) noexcept(true);

    /**
     * Invokes the callback function as soon as the exception of this Future is available.
     * If the exception is already available, the callback function will be executed immediately.
     * If a value is available instead of an exception, the callback function will not be executed.
     * 
     * @param callback Callback function that will be executed as soon as the exception of this Future is available.
     */
    void onException(std::function<void(std::exception)> callback) noexcept(true);


    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> then(std::function<R()> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> then(std::function<std::shared_ptr<R>()> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<R> thenFuture(std::function<Future<R>()> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as the value of this Future is available.
     * 
     * @tparam R Type of the new Future.
     * @param callback Callback function that will be executed as soon as the value of this Future is available.
     * @return Future<std::shared_ptr<R>> Future that will be set to the result of the callback function.
     */
    template<typename R>
    Future<std::shared_ptr<R>> thenFuture(std::function<Future<std::shared_ptr<R>>()> callback) noexcept(true);


    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<void> catchAll(std::function<void(std::exception)> callback) noexcept(true);

    /**
     * Returns a new Future that will be set to the result of the callback function.
     * The callback function will be executed as soon as an exception is available for this Future.
     * 
     * @param callback Callback function that will be executed as soon as an exception is available for this Future.
     * @return Future<R> Future that will be set to the result of the callback function.
     */
    Future<void> catchAllFuture(std::function<Future<void>(std::exception)> callback) noexcept(true);
};





// ---------------------------------------------------------------------
// ----- [ STATE DECLARATION ] -----------------------------------------
// ---------------------------------------------------------------------


template<typename T>
class PromiseFutureState {
public:

    // referencens
    std::atomic<size_t> refs = 1; // number of promises & futures pointing to this state
    std::atomic<size_t> promiseRefs = 0; // number of promises pointing to this state

    // state
    std::mutex mState;
    std::condition_variable cvReady; // used when thread wants to wait for value
    bool ready = false;
    std::optional<T> value;
    std::optional<std::exception> exception;

    // callbacks
    std::mutex mCallbacks; // need own mutex because callbacks want to access state
    std::vector<std::function<void(PromiseFutureState<T>*)>> callbacks; // used when callback wants to wait for value
};

template<typename T>
class PromiseFutureState<std::shared_ptr<T>> {
public:

    // referencens
    std::atomic<size_t> refs = 1; // number of promises & futures pointing to this state
    std::atomic<size_t> promiseRefs = 0; // number of promises pointing to this state

    // state
    std::mutex mState;
    std::condition_variable cvReady; // used when thread wants to wait for value
    bool ready = false;
    std::shared_ptr<T> value;
    std::optional<std::exception> exception;

    // callbacks
    std::mutex mCallbacks; // need own mutex because callbacks want to access state
    std::vector<std::function<void(PromiseFutureState<std::shared_ptr<T>>*)>> callbacks; // used when callback wants to wait for value
};

template<>
class PromiseFutureState<void> {
public:

    // referencens
    std::atomic<size_t> refs = 1; // number of promises & futures pointing to this state
    std::atomic<size_t> promiseRefs = 0; // number of promises pointing to this state

    // state
    std::mutex mState;
    std::condition_variable cvReady; // used when thread wants to wait for value
    bool ready = false;
    std::optional<std::exception> exception;

    // callbacks
    std::mutex mCallbacks; // need own mutex because callbacks want to access state
    std::vector<std::function<void(PromiseFutureState<void>*)>> callbacks; // used when callback wants to wait for value
};





// ---------------------------------------------------------------------
// ----- [ PROMISE IMPLEMENTATION ] ------------------------------------
// ---------------------------------------------------------------------


template<typename T>
Promise<T>::Promise(){
    this->state = new PromiseFutureState<T>();
    state->promiseRefs++;
}

template<typename T>
Promise<T>::Promise(Promise<T> &&other){
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
}

template<typename T>
Promise<T>& Promise<T>::operator=(Promise<T> &&other) {
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
    return *this;
}

template<typename T>
Promise<T>::~Promise() noexcept(false) {
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

template<typename T>
bool Promise<T>::isFulfilled() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

template<typename T>
void Promise<T>::set_value(T value) noexcept(false) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready)
        throw std::runtime_error("Promise already fulfilled");
    state->ready = true;
    state->value = value;
    lockState.unlock();

    state->cvReady.notify_all();

    std::lock_guard<std::mutex> lockCallbacks(state->mCallbacks);
    for(auto &callback : state->callbacks)
        callback(state);
    state->callbacks.clear();
}

template<typename T>
void Promise<T>::set_exception(const std::exception &exception) noexcept(false) {
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

template<typename T>
Future<T> Promise<T>::get_future() {
    return Future<T>(state); // future will add itself to the state
}



template<typename T>
Promise<std::shared_ptr<T>>::Promise(){
    this->state = new PromiseFutureState<std::shared_ptr<T>>();
    state->promiseRefs++;
}

template<typename T>
Promise<std::shared_ptr<T>>::Promise(Promise<std::shared_ptr<T>> &&other){
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
}

template<typename T>
Promise<std::shared_ptr<T>>& Promise<std::shared_ptr<T>>::operator=(Promise<std::shared_ptr<T>> &&other) {
    this->state = other.state;
    state->refs++;
    state->promiseRefs++;
    return *this;
}

template<typename T>
Promise<std::shared_ptr<T>>::~Promise() noexcept(false) {
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

template<typename T>
bool Promise<std::shared_ptr<T>>::isFulfilled() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

template<typename T>
void Promise<std::shared_ptr<T>>::set_value(std::shared_ptr<T> value) noexcept(false) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready)
        throw std::runtime_error("Promise already fulfilled");
    state->ready = true;
    state->value = value;
    lockState.unlock();

    state->cvReady.notify_all();

    std::lock_guard<std::mutex> lockCallbacks(state->mCallbacks);
    for(auto &callback : state->callbacks)
        callback(state);
    state->callbacks.clear();
}

template<typename T>
void Promise<std::shared_ptr<T>>::set_exception(const std::exception &exception) noexcept(false) {
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

template<typename T>
Future<std::shared_ptr<T>> Promise<std::shared_ptr<T>>::get_future() {
    return Future<std::shared_ptr<T>>(state); // future will add itself to the state
}





// ---------------------------------------------------------------------
// ----- [ FUTURE IMPLEMENTATION ] -------------------------------------
// ---------------------------------------------------------------------

template<typename T>
Future<T>::Future(T value){
    this->state = new PromiseFutureState<T>();
    state->ready = true;
    state->value = value;
}

template<typename T>
Future<T>::Future(const std::exception &exception){
    this->state = new PromiseFutureState<T>();
    state->ready = true;
    state->exception = exception;
}

template<typename T>
Future<T>::Future(PromiseFutureState<T> *state) {
    this->state = state;
    state->refs++;
}

template<typename T>
Future<T>::Future(const Future<T> &other){
    this->state = other.state;
    state->refs++;
}

template<typename T>
Future<T>& Future<T>::operator=(const Future<T> &other){
    if(this != &other){
        this->state = other.state;
        state->refs++;
    }
    return *this;
}

template<typename T>
Future<T>::Future(Future<T> &&other) {
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
}

template<typename T>
Future<T>& Future<T>::operator=(Future<T> &&other){
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
    return *this;
}

template<typename T>
Future<T>::~Future(){
    if(state->refs.fetch_sub(1) == 1){
        delete state; // no more references to this state
    }
}

template<typename T>
bool Future<T>::is_ready() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

template<typename T>
bool Future<T>::has_value() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->value.has_value();
}

template<typename T>
bool Future<T>::has_exception() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->exception.has_value();
}

template<typename T>
T Future<T>::get_value() noexcept(false) {
    std::unique_lock<std::mutex> lock(state->mState);
    if(state->value.has_value()){
        return state->value.value();
    } else if(state->exception.has_value()){
        throw state->exception.value();
    } else {
        // wait for ready
        state->cvReady.wait(lock, [this]{ return state->ready; });

        if(state->value.has_value())
            return state->value.value();
        throw state->exception.value();
    }
}

template<typename T>
std::exception Future<T>::get_exception() noexcept(false) {
    std::unique_lock<std::mutex> lock(state->mState);
    if(state->exception.has_value()) {
        return state->exception.value();
    } else if(state->value.has_value()) {
        throw std::runtime_error("Future has value instead of exception");
    } else {
        // wait for ready
        state->cvReady.wait(lock, [this]{ return state->ready; });

        if(state->exception.has_value())
            return state->exception.value();
        throw std::runtime_error("Future has value instead of exception");
    }
}


template<typename T>
void Future<T>::onValue(std::function<void(T)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            T value = state->value.value();
            lockState.unlock(); // unlock before callback
            callback(value);
            return;
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback(value);
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<T> *state){
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                callback(value);
            }
        });
    }
}

template<typename T>
void Future<T>::onException(std::function<void(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->exception.has_value()){
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            callback(exception);
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
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback(exception);
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<T> *state){
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->exception.has_value()){
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                callback(exception);
            }
        });
    }
}


// callback taking T and returning R/void
template<typename T>
template<typename R>
Future<R> Future<T>::then(std::function<R(T)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            T value = state->value.value();
            lockState.unlock(); // unlock before callback
            try {
                if constexpr (std::is_same<R, void>::value) {
                    callback(value);
                    return Future<R>();
                } else {
                    return Future<R>(callback(value));
                }
            } catch(const std::exception &exception) {
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
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback(value);
                        return Future<R>();
                    } else {
                        return Future<R>(callback(value));
                    }
                } catch(const std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback(value);
                        promise->set_value();
                    } else {
                        promise->set_value(callback(value));
                    }
                } catch (const std::exception &exception) {
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

// callback taking T and returning shared_ptr<R>
template<typename T>
template<typename R>
Future<std::shared_ptr<R>> Future<T>::then(std::function<std::shared_ptr<R>(T)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            T value = state->value.value();
            lockState.unlock(); // unlock before callback
            try {
                return Future<std::shared_ptr<R>>(callback(value));
            } catch(const std::exception &exception) {
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
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return Future<std::shared_ptr<R>>(callback(value));
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                try {
                    promise->set_value(callback(value));
                } catch(const std::exception &exception) {
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

// callback taking T and returning Future<R>/Future<void>
template<typename T>
template<typename R>
Future<R> Future<T>::thenFuture(std::function<Future<R>(T)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            T value = state->value.value();
            lockState.unlock(); // unlock before callback
            try {
                return callback(value);
            } catch(const std::exception &exception) {
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
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(value);
                } catch(const std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                try {
                    Future<R> future = callback(value);
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
                } catch(const std::exception &exception) {
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

// callback taking T and returning Future<shared_ptr<R>>
template<typename T>
template<typename R>
Future<std::shared_ptr<R>> Future<T>::thenFuture(std::function<Future<std::shared_ptr<R>>(T)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            T value = state->value.value();
            lockState.unlock(); // unlock before callback
            try {
                return callback(value);
            } catch(const std::exception &exception) {
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
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(value);
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before callback
                try {
                    Future<std::shared_ptr<R>> future = callback(value);
                    future.onValue([promise](std::shared_ptr<R> value){
                        promise->set_value(value);
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (const std::exception &exception) {
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


// callback taking exception and returning T
template<typename T>
Future<T> Future<T>::catchAll(std::function<T(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            return Future<T>(state->value.value());
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                return Future<T>(callback(exception));
            } catch(const std::exception &exception) {
                return Future<T>(exception);
            }
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(state->value.has_value()){
                return Future<T>(state->value.value());
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return Future<T>(callback(exception));
                } catch(const std::exception &exception) {
                    return Future<T>(exception);
                }
            }
        }

        // wait for this ready
        Promise<T> *promise = new Promise<T>();
        Future<T> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before setting value
                promise->set_value(value);
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    promise->set_value(callback(exception));
                } catch(const std::exception &exception) {
                    promise->set_exception(exception);
                }
            }
            delete promise;
        });
        return future;
    }
}

// callback taking exception and returning Future<T>
template<typename T>
Future<T> Future<T>::catchAllFuture(std::function<Future<T>(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->value.has_value()){
            return Future<T>(state->value.value());
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                return callback(exception);
            } catch(const std::exception &exception) {
                return Future<T>(exception);
            }
        }
    } else {
        // wait for this ready
        lockState.unlock();
        std::unique_lock<std::mutex> lockCallbacks(state->mCallbacks, std::defer_lock);
        std::lock(lockState, lockCallbacks); // lock mState and mCallbacks simultaneously

        // edge case where state became ready while waiting for locks
        if(state->ready){
            if(state->value.has_value()){
                return Future<T>(state->value.value());
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(exception);
                } catch(const std::exception &exception) {
                    return Future<T>(exception);
                }
            }
        }

        // wait for this ready
        Promise<T> *promise = new Promise<T>();
        Future<T> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<T> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->value.has_value()){
                T value = state->value.value();
                lockState.unlock(); // unlock before setting value
                promise->set_value(value);
                delete promise;
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    Future<T> future = callback(exception);
                    future.onValue([promise](T value){
                        promise->set_value(value);
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch(const std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            }
        });
        return future;
    }
}





template<typename T>
Future<std::shared_ptr<T>>::Future(std::shared_ptr<T> value){
    this->state = new PromiseFutureState<std::shared_ptr<T>>();
    state->ready = true;
    state->value = value;
}

template<typename T>
Future<std::shared_ptr<T>>::Future(const std::exception &exception){
    this->state = new PromiseFutureState<std::shared_ptr<T>>();
    state->ready = true;
    state->exception = exception;
}

template<typename T>
Future<std::shared_ptr<T>>::Future(PromiseFutureState<std::shared_ptr<T>> *state) {
    this->state = state;
    state->refs++;
}

template<typename T>
Future<std::shared_ptr<T>>::Future(const Future<std::shared_ptr<T>> &other){
    this->state = other.state;
    state->refs++;
}

template<typename T>
Future<std::shared_ptr<T>>& Future<std::shared_ptr<T>>::operator=(const Future<std::shared_ptr<T>> &other){
    if(this != &other){
        this->state = other.state;
        state->refs++;
    }
    return *this;
}

template<typename T>
Future<std::shared_ptr<T>>::Future(Future<std::shared_ptr<T>> &&other) {
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
}

template<typename T>
Future<std::shared_ptr<T>>& Future<std::shared_ptr<T>>::operator=(Future<std::shared_ptr<T>> &&other){
    this->state = other.state;
    state->refs++;
    // other future will remove itself from state when it gets deleted
    return *this;
}

template<typename T>
Future<std::shared_ptr<T>>::~Future(){
    if(state->refs.fetch_sub(1) == 1){
        delete state; // no more references to this state
    }
}

template<typename T>
bool Future<std::shared_ptr<T>>::is_ready() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready;
}

template<typename T>
bool Future<std::shared_ptr<T>>::has_value() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->ready && !state->exception.has_value();
}

template<typename T>
bool Future<std::shared_ptr<T>>::has_exception() const {
    std::lock_guard<std::mutex> lock(state->mState);
    return state->exception.has_value();
}

template<typename T>
std::shared_ptr<T> Future<std::shared_ptr<T>>::get_value() noexcept(false) {
    std::unique_lock<std::mutex> lock(state->mState);
    if(state->ready && !state->exception.has_value()){
        return state->value;
    } else if(state->exception.has_value()){
        throw state->exception.value();
    } else {
        // wait for ready
        state->cvReady.wait(lock, [this]{ return state->ready; });
        
        if(state->ready && !state->exception.has_value())
            return state->value;
        throw state->exception.value();
    }
}

template<typename T>
std::exception Future<std::shared_ptr<T>>::get_exception() noexcept(false) {
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


template<typename T>
void Future<std::shared_ptr<T>>::onValue(std::function<void(std::shared_ptr<T>)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            std::shared_ptr<T> value = state->value;
            lockState.unlock(); // unlock before callback
            callback(value);
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
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback(value);
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<std::shared_ptr<T>> *state){
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                callback(value);
            }
        });
    }
}

template<typename T>
void Future<std::shared_ptr<T>>::onException(std::function<void(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(state->exception.has_value()){
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            callback(exception);
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
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                callback(exception);
                return;
            }
        }

        // wait for this ready
        state->callbacks.push_back([callback](PromiseFutureState<std::shared_ptr<T>> *state){
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(state->exception.has_value()){
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                callback(exception);
            }
        });
    }
}


// callback taking shared_ptr<T> and returning R/void
template<typename T>
template<typename R>
Future<R> Future<std::shared_ptr<T>>::then(std::function<R(std::shared_ptr<T>)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            std::shared_ptr<T> value = state->value;
            lockState.unlock(); // unlock before callback
            try {
                if constexpr (std::is_same<R, void>::value) {
                    callback(value);
                    return Future<R>();
                } else {
                    return Future<R>(callback(value));
                }
            } catch(const std::exception &exception) {
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
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback(value);
                        return Future<R>();
                    } else {
                        return Future<R>(callback(value));
                    }
                } catch(const std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                try {
                    if constexpr (std::is_same<R, void>::value) {
                        callback(value);
                        promise->set_value();
                    } else {
                        promise->set_value(callback(value));
                    }
                } catch(const std::exception &exception) {
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

// callback taking shared_ptr<T> and returning shared_ptr<R>
template<typename T>
template<typename R>
Future<std::shared_ptr<R>> Future<std::shared_ptr<T>>::then(std::function<std::shared_ptr<R>(std::shared_ptr<T>)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            std::shared_ptr<T> value = state->value;
            lockState.unlock(); // unlock before callback
            try {
                return Future<std::shared_ptr<R>>(callback(value));
            } catch(const std::exception &exception) {
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
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return Future<std::shared_ptr<R>>(callback(value));
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                try {
                    promise->set_value(callback(value));
                } catch(const std::exception &exception) {
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

// callback taking shared_ptr<T> and returning Future<R>/Future<void>
template<typename T>
template<typename R>
Future<R> Future<std::shared_ptr<T>>::thenFuture(std::function<Future<R>(std::shared_ptr<T>)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            std::shared_ptr<T> value = state->value;
            lockState.unlock(); // unlock before callback
            try {
                return callback(value);
            } catch(const std::exception &exception) {
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
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(value);
                } catch (const std::exception &exception) {
                    return Future<R>(exception);
                }
            } else {
                return Future<R>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<R> *promise = new Promise<R>();
        Future<R> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                try {
                    Future<R> future = callback(value);
                    if constexpr (std::is_same<R, void>::value) {
                        future.onValue([promise]() -> void {
                            promise->set_value();
                            delete promise;
                        });
                    } else {
                        future.onValue([promise](R value) {
                            promise->set_value(value);
                            delete promise;
                        });
                    }
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch(const std::exception &exception) {
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

// callback taking shared_ptr<T> and returning Future<shared_ptr<R>>
template<typename T>
template<typename R>
Future<std::shared_ptr<R>> Future<std::shared_ptr<T>>::thenFuture(std::function<Future<std::shared_ptr<R>>(std::shared_ptr<T>)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            std::shared_ptr<T> value = state->value;
            lockState.unlock(); // unlock before callback
            try {
                return callback(value);
            } catch(const std::exception &exception) {
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
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(value);
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<R>>(exception);
                }
            } else {
                return Future<std::shared_ptr<R>>(state->exception.value());
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<R>> *promise = new Promise<std::shared_ptr<R>>();
        Future<std::shared_ptr<R>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before callback
                try {
                    Future<std::shared_ptr<R>> future = callback(value);
                    future.onValue([promise](std::shared_ptr<R> value){
                        promise->set_value(value);
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch (const std::exception &exception) {
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
template<typename T>
Future<std::shared_ptr<T>> Future<std::shared_ptr<T>>::catchAll(std::function<std::shared_ptr<T>(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            return Future<std::shared_ptr<T>>(state->value);
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                return Future<std::shared_ptr<T>>(callback(exception));
            } catch(const std::exception &exception) {
                return Future<std::shared_ptr<T>>(exception);
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
                return Future<std::shared_ptr<T>>(state->value);
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return Future<std::shared_ptr<T>>(callback(exception));
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<T>>(exception);
                }
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<T>> *promise = new Promise<std::shared_ptr<T>>();
        Future<std::shared_ptr<T>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before setting value
                promise->set_value(value);
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    promise->set_value(callback(exception));
                } catch(const std::exception &exception) {
                    promise->set_exception(exception);
                }
            }
            delete promise;
        });
        return future;
    }
}

// callback taking exception and returning Future<shared_ptr<T>>
template<typename T>
Future<std::shared_ptr<T>> Future<std::shared_ptr<T>>::catchAllFuture(std::function<Future<std::shared_ptr<T>>(std::exception)> callback) noexcept(true) {
    std::unique_lock<std::mutex> lockState(state->mState);
    if(state->ready){
        if(!state->exception.has_value()){
            return Future<std::shared_ptr<T>>(state->value);
        } else {
            std::exception exception = state->exception.value();
            lockState.unlock(); // unlock before callback
            try {
                return callback(exception);
            } catch(const std::exception &exception) {
                return Future<std::shared_ptr<T>>(exception);
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
                return Future<std::shared_ptr<T>>(state->value);
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                lockCallbacks.unlock(); // unlock before callback
                try {
                    return callback(exception);
                } catch(const std::exception &exception) {
                    return Future<std::shared_ptr<T>>(exception);
                }
            }
        }

        // wait for this ready
        Promise<std::shared_ptr<T>> *promise = new Promise<std::shared_ptr<T>>();
        Future<std::shared_ptr<T>> future = promise->get_future();
        state->callbacks.push_back([promise, callback](PromiseFutureState<std::shared_ptr<T>> *state){ // this future may be already deleted when invoked
            // this state ready
            std::unique_lock<std::mutex> lockState(state->mState);
            if(!state->exception.has_value()){
                std::shared_ptr<T> value = state->value;
                lockState.unlock(); // unlock before setting value
                promise->set_value(value);
                delete promise;
            } else {
                std::exception exception = state->exception.value();
                lockState.unlock(); // unlock before callback
                try {
                    Future<std::shared_ptr<T>> future = callback(exception);
                    future.onValue([promise](std::shared_ptr<T> value){
                        promise->set_value(value);
                        delete promise;
                    });
                    future.onException([promise](std::exception exception){
                        promise->set_exception(exception);
                        delete promise;
                    });
                } catch(const std::exception &exception) {
                    promise->set_exception(exception);
                    delete promise;
                }
            }
        });
        return future;
    }
}

}
#endif // SPI_FUTURE_HPP
