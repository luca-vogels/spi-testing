#include "./utils/Future.hpp"

#include <memory>

using namespace spi;


int main() {

    // future returning value
    auto num1 = Future<std::string>("test").then<int>([](std::string str) {
        return (int)str.size();
    });

    // future returning future (wasn't possible)
    auto num2 = Future<std::string>("test").thenFuture<int>([](std::string str) {
        return Future<int>((int)str.size());
    });

    // future returning void
    auto num3 = Future<std::string>("test").then<void>([](std::string str) {
        (void)str;
        return;
    });

    // future returning future void
    auto num4 = Future<std::string>("test").thenFuture<void>([](std::string str) {
        (void)str;
        return Future<void>();
    });

    // future returning shared_ptr<int>
    auto num5 = Future<std::string>("test").then<std::shared_ptr<int>>([](std::string str) {
        return std::make_shared<int>(str.size());
    });

    // future returning future shared_ptr<int> (wasn't possible)
    auto num6 = Future<std::string>("test").thenFuture<std::shared_ptr<int>>([](std::string str) {
        return Future<std::shared_ptr<int>>(std::make_shared<int>(str.size()));
    });

    // future returning nested futures
    auto num7 = Future<std::string>("test").thenFuture<int>([](std::string str) {
        Promise<int> *promise = new Promise<int>();
        Future<int> future = promise->get_future();
        Future<std::string>(str).then<void>([promise](std::string str) {
            promise->set_value((int)str.size());
        });
        return future;
    });

    return 0;
}
