from asyncio import new_event_loop
from time import sleep
from threading import Thread

import eider

from eider_pybind11_test import DuckTester, DuckTestFactory


DHOW = 'like a duck'
DUCK = {'looks': DHOW, 'swims': DHOW, 'quacks': DHOW}

GHOW = 'like a goose'
GOOSE = {'looks': GHOW, 'swims': GHOW, 'quacks': GHOW}

COW = {'eats': 'like a cow'}


PORT_PY = 12345
PORT_CPP = 12346


class PyDuckTestFactory(eider.LocalRoot):
    # Demonstrates that pybind11 objects can be returned by pure-Python roots.

    # Note: the _newables magic doesn't work with pybind11 classes because
    # inspect.signature() doesn't grok their __init__ methods.

    def new_DuckTester(self):
        return DuckTester(self._lsession)


def do_the_tests(port, root):
    t = Thread(target=eider.serve,
               args=[port, new_event_loop()],
               kwargs={'root': root, 'handle_signals': False},
               daemon=True)
    t.start()

    # Wait a bit to make sure the server is established before we try to
    # connect to it.
    sleep(0.1)

    url = 'ws://localhost:{}/'.format(port)
    with eider.BlockingConnection(url) as conn:
        with conn.create_session() as rroot:
            dt = rroot.new_DuckTester()
            assert dt.is_it_a_duck(DUCK)
            assert not dt.is_it_a_duck(GOOSE)
            try:
                dt.is_it_a_duck(COW)
            except KeyError:
                pass
            else:
                assert False, "that's an odd duck"


def test_pyroot():
    do_the_tests(PORT_PY, PyDuckTestFactory)


def test_cpproot():
    do_the_tests(PORT_CPP, DuckTestFactory)
