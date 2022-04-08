#pragma once

namespace estd { namespace pmr {

namespace experimental {

struct memory_resource
{
    void* do_allocate(std::size_t bytes, std::size_t alignment) = 0;
};

struct unsynchronized_pool_resource : memory_resource
{

};

}

}}