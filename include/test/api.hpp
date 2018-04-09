// This is a simple API in plain C++ that knows nothing of Eider or pybind11.

#pragma once

#include <memory>  // std::shared_ptr
#include <string>  // std::string

namespace api {


// This allows us to track the lifetime of species (i.e. make sure they're
// being properly destructed.)
struct Counter {
    int _n;
    Counter() : _n{0} {}
};


struct Species {
    std::string _how;  // description of how this thing behaves
    std::shared_ptr<Counter> _nspecies;  // global instance counter

    Species(std::string how, std::shared_ptr<Counter> nspecies) :
        _how{how}, _nspecies{nspecies}
    {
        ++nspecies->_n;
    }

    ~Species() {
        --_nspecies->_n;
    }
};


struct Animal : Species {
    using Species::Species;

    std::string look() {
        return _how;
    }

    std::string swim() {
        return _how;
    }

    std::string quack() {
        return _how;
    }
};


struct Plant : Species {
    using Species::Species;

    std::string grow() {
        return _how;
    }
};


struct Ecosystem {
    std::shared_ptr<api::Counter> _nspecies;

    Ecosystem() :
        _nspecies{std::make_shared<api::Counter>()}
    {
    }

    bool duck_test(Animal &a, std::string how) {
        return a.look() == how && a.swim() == how && a.quack() == how;
    }

    int biodiversity() {
        return _nspecies->_n;
    }
};

} // namespace api
