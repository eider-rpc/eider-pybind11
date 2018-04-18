#include "test/api_eider.hpp"

namespace eider = eider_pybind11;
namespace py = eider::py;

using api_eider::Animal;
using api_eider::Plant;
using api_eider::Ecosystem;


// Expose the plain C++ API via pybind11.
static void bind_api(py::handle m) {
    // PyEcosystem needs this.
    py::class_<api::Counter, std::shared_ptr<api::Counter>>(m, "Counter")
        .def(py::init<>())
        .def("get", [](api::Counter &c) { return c._n; });

    // Needed to demonstrate automatic direct property access.
    py::class_<api::Species>(m, "Species")
        .def_readwrite("how", &api::Species::how);

    // None of the tests use the other plain API classes directly; however, we
    // need the "forward declaration" of api::Animal in order for pybind11 to
    // properly typecheck and downcast the argument to Ecosystem::duck_test().
    py::class_<api::Animal, api::Species>(m, "RawAnimal");
    py::class_<api::Plant, api::Species>(m, "RawPlant");
    py::class_<api::Ecosystem>(m, "RawEcosystem");
}


// Expose the API wrappers for Eider support.
static void bind_api_eider(py::handle m) {
    // Note the initial eider::LocalSession argument to py::init<>().  Without
    // this, the classes' Python constructors would not operate correctly (see
    // eider::LocalSession::init_object() for details).

    py::class_<Animal, eider::LocalObject, api::Animal>(m, "Animal")
        .def(py::init<
            eider::LocalSession, std::string, std::shared_ptr<api::Counter>>())
        .def("look", &Animal::look)
        .def("swim", &Animal::swim)
        .def("quack", &Animal::quack);

    py::class_<Plant, eider::LocalObject, api::Plant>(m, "Plant")
        .def(py::init<
            eider::LocalSession, std::string, std::shared_ptr<api::Counter>>())
        .def("grow", &Plant::grow);

    py::class_<Ecosystem, eider::LocalRoot, api::Ecosystem>(m, "Ecosystem")
        .def(py::init<eider::LocalSession>())
        .def("new_Animal", &Ecosystem::new_Animal)
        .def("new_Plant", &Ecosystem::new_Plant)
        .def("duck_test", &Ecosystem::duck_test)
        .def("biodiversity", &Ecosystem::biodiversity);
}


PYBIND11_MODULE(eider_pybind11_test, m) {
    bind_api(m);
    eider::bind(m);
    bind_api_eider(m);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
