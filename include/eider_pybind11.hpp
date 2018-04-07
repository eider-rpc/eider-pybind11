// Copyright 2017 Semiconductor Components Industries LLC (d/b/a "ON
// Semiconductor")
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <pybind11/pybind11.h>

namespace eider_pybind11 {

namespace py = pybind11;

using namespace py::literals;


struct LocalObjectBase {
    py::object _lsession;
    py::object _loid;
    py::object _lref;
    int _nref;

    // We must break the constructor into two stages because the vptr must be
    // set up before we can call py::cast(this).
    LocalObjectBase(py::object lsession, int nref) :
        _lsession{lsession}, _nref{nref}
    {
    }

    void init(py::object loid) {
        _loid = loid;
        _lref = py::dict("__*__"_a=loid, "lsid"_a=_lsession.attr("lsid"));
    }

    virtual ~LocalObjectBase() {
    }

    void release() {
        if (--_nref <= 0) {
            _release();
        }
    }

    void _release() {
        if (PyObject_DelItem(
                _lsession.attr("objects").ptr(),
                _loid.ptr()) == -1) {
            throw py::error_already_set();
        }
        _close();
    }

    virtual void _close() {
        // Break the reference cycle between self and _lsession.  This must be
        // done to avoid memory leaks, because Python's garbage collector can't
        // handle cycles involving extension types.
        _lsession = py::none();
    }

    py::object _marshal() {
        ++_nref;
        return _lref;
    }
};


struct LocalRoot : LocalObjectBase {
    LocalRoot(py::object lsession) :
        LocalObjectBase{lsession, 1}
    {
        LocalObjectBase::init(py::none());
    }

    void init() {
        // This exists merely to allow the LocalSession magic to work.
        // All initialization is performed in the constructor.
    }

    void _close() override {
        _lsession.attr("destroy")();
        LocalObjectBase::_close();
    }
};


struct LocalObject : LocalObjectBase {
    LocalObject(py::object lsession) :
        LocalObjectBase{lsession, 0}
    {
    }

    py::object init() {
        // If my pybind11 wrapper has not yet been created (e.g. I've just been
        // new'd in C++), force the wrapper to take ownership of me, to avoid
        // memory leaks.
        py::object obj = py::cast(
            this, py::return_value_policy::take_ownership);

        // Add me to the local session object lookup table, and store my ID.
        LocalObjectBase::init(_lsession.attr("add")(obj));

        // Return the wrapping python object, so the caller doesn't have to
        // cast me again.
        return obj;
    }
};


struct LocalSession {
    // Placeholder type that allows client code to magically bind the
    // two-stage LocalObject constructor with:
    //      .def(py::init<eider::LocalSession>())
    // This relies on implementation details of pybind11 2.2.

    template <typename Class, typename value_and_holder, typename... Args>
    static void init_object(value_and_holder &v_h,
                            py::object lsession,
                            Args... args) {
        // First stage: construct the C++ type.
        Class *p = new Class(lsession, std::forward<Args>(args)...);

        // Set up the pybind11 wrapper.  
        v_h.value_ptr() = p;
        v_h.type->init_instance(v_h.inst, nullptr);

        // Second stage: add the object to the session.
        p->init();
    }
};


// Call this inside the PYBIND11_MODULE block to expose the Eider base classes.
inline void bind(py::handle m) {
    py::class_<LocalObjectBase>(m, "LocalObjectBase")
        .def("release", &LocalObjectBase::release)
        .def("_release", &LocalObjectBase::_release)
        .def("_close", &LocalObjectBase::_close)
        .def("_marshal", &LocalObjectBase::_marshal);
    py::class_<LocalRoot, LocalObjectBase>(m, "LocalRoot");
    py::class_<LocalObject, LocalObjectBase>(m, "LocalObject");
}

} // namespace eider_pybind11


// Inject partial template specializations into pybind11 to make the
// LocalSession magic work.
namespace pybind11 { namespace detail { namespace initimpl {

template <typename... Args>
struct constructor<eider_pybind11::LocalSession, Args...> {
    template <typename Class, typename... Extra>
    static void execute(Class &cl, const Extra&... extra) {
        cl.def(
            "__init__",
            eider_pybind11::LocalSession::init_object<
                Cpp<Class>,
                value_and_holder,
                Args...>,
            is_new_style_constructor(),
            extra...);
    }
};

}}} // namespace pybind11::detail::initimpl
