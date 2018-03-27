from asyncio import new_event_loop
from threading import Thread

import pytest

import eider

from eider_pybind11_test import TestRoot as Root


PORT = 12345
URL = 'ws://localhost:{}/'.format(PORT)


@pytest.fixture(scope='module')
def server():
    t = Thread(target=eider.serve,
               args=[PORT, new_event_loop()],
               kwargs={'root': Root, 'handle_signals': False},
               daemon=True)
    t.start()


def test_call(server):
    """Create a remote object and call one of its methods."""
    with eider.BlockingConnection(URL) as conn:
        with conn.create_session() as rroot:
            obj = rroot.new_TestObject()
            assert 66 == obj.add(42, 24)
