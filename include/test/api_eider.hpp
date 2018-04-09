// Wrap the API in api.hpp to add Eider support.

#pragma once

#include "test/api.hpp"

#include <eider_pybind11.hpp>

namespace api_eider {

namespace eider = eider_pybind11;
namespace py = eider::py;


struct Animal : eider::LocalObject, api::Animal {
    Animal(py::object lsession, std::string how,
           std::shared_ptr<api::Counter> nspecies) :
        eider::LocalObject{lsession},
        api::Animal{how, nspecies}
    {
    }
};


struct Plant : eider::LocalObject, api::Plant {
    Plant(py::object lsession, std::string how,
          std::shared_ptr<api::Counter> nspecies) :
        eider::LocalObject{lsession},
        api::Plant{how, nspecies}
    {
    }
};


struct Ecosystem : eider::LocalRoot, api::Ecosystem {
    using eider::LocalRoot::LocalRoot;

    py::object new_Animal(std::string how) {
        // Call init() immediately after construction to finish adding the
        // object to the LocalSession.
        return (new Animal(_lsession, how, _nspecies))->init();
    }

    py::object new_Plant(std::string how) {
        return (new Plant(_lsession, how, _nspecies))->init();
    }
};

} // namespace api_eider
