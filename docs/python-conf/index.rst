FlowerEvolver-WASM -- Python API
==================================

Native Python bindings (via `nanobind <https://github.com/wjakob/nanobind>`_)
for FlowerEvolver-WASM - generates 2D/3D flowers from CPPN genomes evolved
via `EvoAI <https://github.com/cristianglezm/EvoAI>`_. Genomes and rendered
images round-trip with the JS/WASM API and the native desktop app -
``Flower.to_json()``/``Flower.from_json()`` use the same wire format
throughout.

For the JavaScript/WASM API (``FEService``, the high-level wrapper most web
consumers want), see the `JS/WASM API docs <../js/>`_. For the underlying
native C++ library, see the `C++ API docs <../cpp/>`_.

Installation and the ``flower-evolver`` command-line tool are covered in
`python/README.md
<https://github.com/cristianglezm/FlowerEvolver-WASM/blob/master/python/README.md>`_
-- this site is the API reference only.

.. toctree::
   :maxdepth: 2

   api

Indices
-------

* :ref:`genindex`
* :ref:`modindex`
