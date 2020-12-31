#include <iostream>
#include <vector>
#include <queue>

#include <patterns/recursive_type_collection.hpp>
#include <patterns/functor.hpp>

using namespace patterns;
using namespace patterns::RCNS;
using namespace patterns::functors;

struct print_smth : public functor<do_nothing_functor_begin, void>
{
    virtual void operator()() override
    {
        std::cout << "smth" << std::endl;
    }
};

struct print_smth_else : public functor<print_smth, void, const std::string&>
{
    virtual void operator()(const std::string& smth_else) override
    {
        std::cout << "printing: " << smth_else << std::endl;
    }
};

using operation_t = functor_collection_t<print_smth_else>;

template<typename functor_t>
using Processor = processors::processor<operation_t, functor_t>;

template<typename functor_t>
using aProcessor = processors::processor_autocall<operation_t, functor_t>;

struct print_processor : public aProcessor<print_smth> {}; // autocall is performed
struct print_smth_else_processor : public Processor<print_smth_else> 
{
    virtual void invoke(operation_t &op) override
    {
        std::get<print_smth_else>(op)("XDDDDDD");
    }
};

using processor_base = processors::processor_base<operation_t>;

int main()
{
    std::queue<operation_t> functors;
    std::vector<processor_base *> processors;

    functors.emplace(print_smth{});
    functors.emplace(print_smth_else{});
    functors.emplace(print_smth_else{});
    functors.emplace(print_smth{});
    functors.emplace(print_smth_else{});

    processors.emplace_back((new print_processor())->get());
    processors.emplace_back((new print_smth_else_processor())->get());
    processors.emplace_back((new processors::not_supported_operation_type_processor<operation_t>())->get());

    while (!functors.empty())
    {
        operation_t f{ std::move( functors.front() )};
        functors.pop();

        for (processor_base *proc : processors)
            if (proc->is_my_type(f))
            {
                proc->invoke(f);
                break;
            }
    }

    for(processor_base* ptr : processors)
        delete ptr;

    return 0;
}