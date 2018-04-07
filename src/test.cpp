#include <eider_pybind11.hpp>

namespace eider = eider_pybind11;
namespace py = eider::py;


struct DuckTester : eider::LocalObject {
    using eider::LocalObject::LocalObject;

    bool is_it_a_duck(py::object obj) {
        return std::string(py::str(obj["looks"])) == "like a duck" &&
            std::string(py::str(obj["swims"])) == "like a duck" &&
            std::string(py::str(obj["quacks"])) == "like a duck";
    }
};


struct DuckTestFactory : eider::LocalRoot {
    using eider::LocalRoot::LocalRoot;

    py::object new_DuckTester() {
        return (new DuckTester(_lsession))->init();
    }
};


PYBIND11_MODULE(eider_pybind11_test, m) {
    eider::bind(m);

    py::class_<DuckTester, eider::LocalObject>(m, "DuckTester")
        .def(py::init<eider::LocalSession>())
        .def("is_it_a_duck", &DuckTester::is_it_a_duck);

    py::class_<DuckTestFactory, eider::LocalRoot>(m, "DuckTestFactory")
        .def(py::init<eider::LocalSession>())
        .def("new_DuckTester", &DuckTestFactory::new_DuckTester);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
