#include "FlowRepresentation.hpp"

using namespace spi;

std::unique_ptr<FlowOutput> FlowOperatorChainable::output() {
    return std::unique_ptr<FlowOutput>(new FlowOutput(this)); // protected constructor
}

std::unique_ptr<FlowMerge> FlowOperatorChainable::merge(std::unique_ptr<FlowOperator> source) {
    std::vector<FlowOperator*> sources = { this, source.release() };
    return std::unique_ptr<FlowMerge>(new FlowMerge(sources)); // protected constructor
}

std::unique_ptr<FlowMerge> FlowOperatorChainable::merge(std::vector<std::unique_ptr<FlowOperator>> sources) {
    std::vector<FlowOperator*> sourcesPtr = { this };
    for(auto& source : sources) sourcesPtr.push_back(source.release());
    return std::unique_ptr<FlowMerge>(new FlowMerge(sourcesPtr)); // protected constructor
}

template<typename EXPRESSION>
std::unique_ptr<FlowFilter<EXPRESSION>> FlowOperatorChainable::filter(EXPRESSION expression) {
    return std::unique_ptr<FlowFilter<EXPRESSION>>(new FlowFilter<EXPRESSION>(this, expression));
}
