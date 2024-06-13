/**
 * Contains all parts to create an abstract flow representation 
 * needed to define a flow.
 * 
 * @file FlowRepresentation.hpp
 * @author Luca Vogels (github@luca-vogels.com)
 */

#ifndef SPI_FLOW_REPRESENTATION_HPP
#define SPI_FLOW_REPRESENTATION_HPP

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

namespace spi {


class FlowOutput; // defer declaration (see below)
class FlowMerge; // defer declaration (see below)
template<typename E> class FlowFilter; // defer declaration (see below)



/**
 * Identifier for a logical expression.
 */
class FlowWindowType {
public:
    enum Type : uint32_t {
        SIZE_BASED,
        TIME_BASED
    };

private:
    Type type;

public:
    FlowWindowType() = default;

    constexpr FlowWindowType(Type type) : type(type) { }

    constexpr operator Type() const { return type; } // switch(type)
    explicit operator bool() const = delete; // prevent if(type)
    bool operator==(FlowWindowType t) const { return t.type == this->type; }
    bool operator!=(FlowWindowType t) const { return t.type != this->type; }
    size_t operator()(const FlowWindowType &t) const { return t.type; }

    std::string toString() const {
        switch(this->type){
            case SIZE_BASED: return "SIZE_BASED";
            case TIME_BASED: return "TIME_BASED";
        } return "UNKNOWN";
    }

    uint32_t toId() const {
        return this->type;
    }

    static const FlowWindowType getById(uint32_t id){
        switch(id){
            case SIZE_BASED: return FlowWindowType::SIZE_BASED;
            case TIME_BASED: return FlowWindowType::TIME_BASED;
        }
        return FlowWindowType::SIZE_BASED;
    }
};



class FlowWindow {
public:

    /**
     * Represents different types of tumbling windows depending on the given size type.
     * 
     * @param type Determines how 'size' and 'stride' are interpreted
     * @param size  TIME_BASED: size of window in milliseconds
     *              SIZE_BASED: size of window in amount of tuples
     */
    FlowWindow(FlowWindowType type, uint64_t size);

    /**
     * Represents different types of windows depending on the given size type.
     * 
     * @param type Determines how 'size' and 'stride' are interpreted
     * @param size  TIME_BASED: size of window in milliseconds
     *              SIZE_BASED: size of window in amount of tuples
     *              SESSION_BASED: max milliseconds between tuples to count to same window
     * @param stride    TIME_BASED: advancement of window in milliseconds (always advances even if no tuples)
     *                  SIZE_BASED: advancement of window in amount of tuples (advances when tuples arrive)
     *                  SESSION_BASED: ignored (extends when tuples arrive within 'size' timeout, otherwise new window)
     */
    FlowWindow(FlowWindowType type, uint64_t size, uint64_t stride);
};






/**
 * Base class for all flow operators.
 */
class FlowOperator {
protected:
    std::unordered_set<FlowOperator*> sources;
    FlowOperator* target = nullptr;

    FlowOperator(){

    }

    FlowOperator(FlowOperator* source) : sources({source}) {
        source->target = this;
    }

    FlowOperator(std::vector<FlowOperator*> sources){
        for(FlowOperator *source : sources){
            this->sources.insert(source);
            source->target = this;
        }
    }

public:

    /** Deleting one operator will cause the whole chain to be deleted */
    ~FlowOperator(){
        if(this->target != nullptr){
            this->target->sources.erase(this);
            delete this->target;
            this->target = nullptr;
        }
        for(FlowOperator *source : this->sources){
            source->target = nullptr;
            delete source;
        }
    }
};



/**
 * Base class for chainable flow operators.
 */
class FlowOperatorChainable : public FlowOperator {
protected:

    FlowOperatorChainable() : FlowOperator() {}

    FlowOperatorChainable(FlowOperator* source) : FlowOperator(source) {}

    FlowOperatorChainable(std::vector<FlowOperator*> sources) : FlowOperator(sources) {}

public:

    /**
     * Signals the end of the flow to which subscribers can subscribe to.
     * 
     * @return std::unique_ptr<FlowOutput> Output of flow needed to define a flow.
     */
    std::unique_ptr<FlowOutput> output();

    /**
     * Merges the output of another source or operator into a single flow.
     * 
     * @param source Source or operators to merge into this flow.
     * @return std::unique_ptr<FlowMerge> Merged flow.
     */
    std::unique_ptr<FlowMerge> merge(std::unique_ptr<FlowOperator> source);

    /**
     * Merges the output of another source or operator into a single flow.
     * 
     * @param source Source or operators to merge into this flow.
     * @return std::unique_ptr<FlowMerge> Merged flow.
     */
    std::unique_ptr<FlowMerge> merge(std::vector<std::unique_ptr<FlowOperator>> sources);

    /**
     * Forwards the output of this operator to a filter operator.
     * 
     * @tparam EXPRESSION Type of expression for filtering.
     * @param expression Expression for filtering.
     * @return std::unique_ptr<FlowFilter<EXPRESSION>> Filter operator this operator is chained to.
     */
    template<typename EXPRESSION>
    std::unique_ptr<FlowFilter<EXPRESSION>> filter(EXPRESSION expression);
};



/**
 * A flow starts with a flow input.
 * It feeds tuples from publishers into the flow.
 */
class FlowInput : public FlowOperatorChainable {
protected:
    std::vector<std::string> workerNames;
    std::vector<std::string> publisherGroups;
    std::vector<std::string> flowNames;

public:

    /**
     * Flow input that takes all tuples from all publishers on all workers and feeds them into the flow.
     */
    FlowInput() : FlowOperatorChainable() {}

    /**
     * Flow input that takes all tuples from the given publisher groups at the given workers 
     * and feeds them into the flow.
     * 
     * If workerNames and publisherGroups are empty, tuples from all publishers on all workers are considered.
     * 
     * @param workerNames Names of the workers the publisher groups 
     *                     are considered (leave empty to consider publisher groups from any worker).
     * @param publisherGroups Publisher groups to consider (leave empty to consider publishers from any group).
     */
    FlowInput(std::vector<std::string> workerNames, std::vector<std::string> publisherGroups) : 
        FlowOperatorChainable(), workerNames(workerNames), publisherGroups(publisherGroups) {}

    /**
     * Flow input that takes all tuples from the output of the given flows and feeds them into this flow.
     * This allows to further process the output of a flow.
     * 
     * @param flowNames Name of the flows to take the output from.
     */
    FlowInput(std::vector<std::string> flowNames) : FlowOperatorChainable(), flowNames(flowNames) {}

};



/**
 * Flow output is the exit point of a flow
 * to which subscribers can subscribe to.
 */
class FlowOutput : public FlowOperator { // not chainable since it is the exit point of a flow
    friend class FlowOperatorChainable; // to access protected constructor
protected:
    FlowOutput(FlowOperator* source) : FlowOperator(source) {}
    
public:

    FlowOutput() = delete;

};



/**
 * Flow merge combines multiple flows into one.
 */
class FlowMerge : public FlowOperatorChainable {
    friend class FlowOperatorChainable; // to access protected constructor
protected:
    FlowMerge(std::vector<FlowOperator*> sources) : FlowOperatorChainable(sources) {}

public:
    
    FlowMerge() = delete;

};



/**
 * Flow filter applies a given expression to each tuple and only lets tuples pass that fulfill the expression.
 * 
 * @tparam E Type of expression (see "Expression.hpp")
 */
template<typename E>
class FlowFilter : public FlowOperatorChainable {
    friend class FlowOperatorChainable; // to access protected constructor
protected:
    E expression;

    FlowFilter(FlowOperator* source, E expression) : FlowOperatorChainable(source), expression(expression) {}

public:

    FlowFilter() = delete;

    /**
     * Returns the expression that is applied to each tuple.
     * 
     * @return E Expression that is applied to each tuple.
     */
    E getExpression() const {
        return this->expression;
    }
};




/**
 * Flow join joins tuples from two sources based on the given expression and window.
 * 
 * @tparam E Type of expression (see "Expression.hpp")
 * @tparam W Type of window (see "Flow.hpp")
 */
template<typename E, typename W>
class FlowJoin : public FlowOperatorChainable {
    friend class FlowOperatorChainable; // to access protected constructor
protected:
    E expression;
    W window;

    FlowJoin(FlowOperator* source, E expression, W window) : FlowOperatorChainable(source), expression(expression), window(window) {}

public:

    FlowJoin() = delete;

    /**
     * Returns the expression that is applied to each tuple.
     * 
     * @return E Expression that is applied to each tuple.
     */
    E getExpression() const {
        return this->expression;
    }

    /**
     * Returns the window that defines how long tuples are considered for joining.
     * 
     * @return W Window that defines how long tuples are considered for joining.
     */
    W getWindow() const {
        return this->window;
    }
};



}
#endif // SPI_FLOW_REPRESENTATION_HPP