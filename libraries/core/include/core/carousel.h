#pragma once

// STL
#include <memory>
#include <thread>

// Project includes
#include <logger/logger.h>
#include "threaded_processor.h"

/*

gets functors, and check for every rergistered `thread_processor` ( = th_p )
if, after visiting, th_p returns true, take next (or wait for new ones) functors
if none of th_p returns true, throw 'not handdled functor'

/*