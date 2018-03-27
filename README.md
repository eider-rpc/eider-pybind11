# eider-pybind11

This is a reimplementation in C++ of those parts of
[eider-py](https://github.com/eider-rpc/eider-py) necessary to create
[pybind11](https://github.com/pybind/pybind11)-based extension modules
containing C++ classes that can be served over
[Eider](http://eider.readthedocs.io/) connections.

Everything you should need is in `include/eider_pybind11.hpp`.

Also included is a simple example (`src/test.cpp`) and test case
(`tests/test_eider_pybind11.py`).  To build and run it:

```sh
python3 -m venv venv
venv/bin/pip install -e .
venv/bin/pip install pytest
venv/bin/py.test
```
