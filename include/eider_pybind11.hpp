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

    virtual ~LocalObjectBase() {
    }

    // This "delayed constructor" is necessary because the vptr must be set up
    // before we can call py::cast(this).
    void init(py::object lsession, py::object loid) {
        _lsession = lsession;
        _loid = loid;
        _lref = py::dict("__*__"_a=loid, "lsid"_a=lsession.attr("lsid"));
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
    }

    py::object _marshal() {
        ++_nref;
        return _lref;
    }
};


struct LocalRoot : LocalObjectBase {
    void init(py::object lsession) {
        LocalObjectBase::init(lsession, py::none());
        _nref = 1;
    }

    void _close() override {
        _lsession.attr("destroy")();
    }
};


struct LocalObject : LocalObjectBase {
    void init(py::object lsession) {
        LocalObjectBase::init(lsession, lsession.attr("add")(py::cast(this)));
        _nref = 0;
    }
};


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
