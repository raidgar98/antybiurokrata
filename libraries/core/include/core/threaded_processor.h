#pragma once

// STL
#include <memory>
#include <thread>

// Project includes
#include <patterns/functor.hpp>
#include <patterns/visitor.hpp>
#include <logger/logger.h>

using patterns::processors::is_valid_processor_t;
using patterns::Visitable;

template<template <class v_t, typename _Supp> typename _Processor > requires is_valid_processor_t< _Processor >
class threaded_processor : public Visitable<threaded_processor<_Processor>>, public Log<threaded_processor<_Processor>>
{
    using thp = threaded_processor;
    using Log<thp<_Processor>>::log;
    using proc_t = _Processor;
    template<typename _Tp>
    using smart_ptr_t = std::unique_ptr<_Tp>;

    smart_ptr_t<_Processor> __m_processor;
    smart_ptr_t<std::jthread> __m_thread;

public:

    // can't be default constructible, nor copyable
    threaded_processor() = delete;
    threaded_processor(const thp&) = delete;
    thp& operator=(const thp&) = delete;

    // only by constructing processor, or by move from existing one thp or processor
    template<typename ... Ts>
    threaded_processor(Ts&&... vargs) : __m_processor{ new _Processor{ std::forward<Ts>(vargs)... } } {}

    threaded_processor(smart_ptr_t<_Processor> ptr) : __m_processor{ std::move( ptr ) } {}
    threaded_processor(thp&& p) : __m_processor{ std::move(p.__m_processor) }, __m_thread{ std::move(p.__m_thread) } {}
    thp& operator=(thp&& p)
    {
        __m_processor = std::move( p.__m_processor );
        __m_thread = std::move( p.__m_thread );
    }

/*

visitor: checks is it processable by this processor
if so: returns true, and moves it to new thread
if no: returns false

*/

};