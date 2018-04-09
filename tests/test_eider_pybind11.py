from asyncio import new_event_loop
from time import sleep
from threading import Thread

import eider

from eider_pybind11_test import Animal, Counter, Plant, Ecosystem


DHOW = 'like a duck'
GHOW = 'like a goose'
THOW = 'like a tree'


PORT_PY = 12345
PORT_CPP = 12346


class PyEcosystem(eider.LocalRoot):
    # Pure-Python version of Ecosystem.  Demonstrates that pybind11 objects can
    # be created by Python-based roots.

    def __init__(self, lsession):
        eider.LocalRoot.__init__(self, lsession)
        self._nspecies = Counter()

    # Note: the _newables magic doesn't work with pybind11 classes because
    # inspect.signature() doesn't grok their __init__ methods.

    def new_Animal(self, how):
        return Animal(self._lsession, how, self._nspecies)

    def new_Plant(self, how):
        return Plant(self._lsession, how, self._nspecies)

    def duck_test(self, a, how):
        return a.look() == how and a.swim() == how and a.quack() == how

    def biodiversity(self):
        return self._nspecies.get()


def do_the_tests(port, root, Exc):
    # Serve the API from a separate thread, so we can use BlockingConnection in
    # this one.
    t = Thread(target=eider.serve,
               args=[port, new_event_loop()],
               kwargs={'root': root, 'handle_signals': False},
               daemon=True)
    t.start()

    # Wait a bit to make sure the server is established.
    sleep(0.1)

    # Connect to the server and run it through its paces.
    url = 'ws://localhost:{}/'.format(port)
    with eider.BlockingConnection(url) as conn:
        with conn.create_session() as env:
            assert env.biodiversity() == 0

            # Use with-statements to ensure deterministic object destruction.
            # This lets us use biodiversity() to verify that refcounting is
            # being done properly and C++ objects are not being leaked.
            with env.new_Animal(DHOW) as duck:
                assert env.biodiversity() == 1

                # Ducks are ducks.
                assert duck.quack() == DHOW
                assert env.duck_test(duck, DHOW)

                # Geese are animals, but are not ducks.
                with env.new_Animal(GHOW) as goose:
                    assert env.biodiversity() == 2
                    assert not env.duck_test(goose, DHOW)
                assert env.biodiversity() == 1

                # Trees aren't even animals.
                with env.new_Plant(THOW) as tree:
                    assert env.biodiversity() == 2
                    try:
                        env.duck_test(tree, DHOW)
                    except Exc:
                        pass
                    else:
                        assert False, "that's an odd duck"
                assert env.biodiversity() == 1

            assert env.biodiversity() == 0


def test_pyroot():
    # duck_test() will raise AttributeError if argument does not have look()
    do_the_tests(PORT_PY, PyEcosystem, AttributeError)


def test_cpproot():
    # duck_test() will raise TypeError if argument is not an Animal
    do_the_tests(PORT_CPP, Ecosystem, TypeError)
