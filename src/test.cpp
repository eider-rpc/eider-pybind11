#include <eider_pybind11.hpp>

namespace eider = eider_pybind11;
namespace py = eider::py;


struct TestObject : eider::LocalObject {
    TestObject(py::object lsession) {
        LocalObject::init(lsession);
    }

    int add(int i, int j) {
        return i + j;
    }
};


struct TestRoot : eider::LocalRoot {
    TestRoot(py::object lsession) {
        LocalRoot::init(lsession);
    }

    py::object new_TestObject() {
        return py::cast(new TestObject(_lsession));
    }
};


PYBIND11_MODULE(eider_pybind11_test, m) {
    eider::bind(m);
    
    py::class_<TestObject, eider::LocalObject>(m, "TestObject")
        .def(py::init<py::object>())
        .def("add", &TestObject::add);

    py::class_<TestRoot, eider::LocalRoot>(m, "TestRoot")
        .def(py::init<py::object>())
        .def("new_TestObject", &TestRoot::new_TestObject);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
