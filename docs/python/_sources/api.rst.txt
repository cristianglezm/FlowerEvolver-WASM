API Reference
=============

flowerevolver
-------------

Every class and free function here is bound directly from the C++ core
(``src/bindings/python/*.cpp``) via nanobind - the same code path the
JS/WASM API's ``FEService`` and the native C++ library's
``include/fe/FlowerEvolver.hpp`` both go through, so behavior (and, for
``Flower``/``DNA``'s JSON, wire format) matches across all three.

.. automodule:: flowerevolver

Free functions
~~~~~~~~~~~~~~

.. autofunction:: flowerevolver.make_flower
.. autofunction:: flowerevolver.make_petals
.. autofunction:: flowerevolver.make_petal_layer
.. autofunction:: flowerevolver.make_stem
.. autofunction:: flowerevolver.draw_flower
.. autofunction:: flowerevolver.draw_petals
.. autofunction:: flowerevolver.draw_petal_layer
.. autofunction:: flowerevolver.reproduce
.. autofunction:: flowerevolver.mutate
.. autofunction:: flowerevolver.make_3d_flower
.. autofunction:: flowerevolver.get_flower_stats
.. autofunction:: flowerevolver.get_version
.. autofunction:: flowerevolver.get_commit_hash
.. autofunction:: flowerevolver.get_version_string

flowerevolver.cli
------------------

The ``flower-evolver`` console script installed alongside the package -
matches the native desktop app's own single-flower flags. See
the module docstring below for the exact invocations; ``flower-evolver
--help`` covers the same ground interactively.

.. cli.py imports Flower/make_flower/mutate/reproduce from the package
   itself for its own use (see its own source) -- conf.py's
   autodoc_default_options turns on imported-members globally (needed for
   flowerevolver's own __init__.py re-exports above), so without this
   override those four would get redundantly re-documented here too,
   under flowerevolver.cli, producing a "duplicate object description"
   warning against the copy already documented above.
.. automodule:: flowerevolver.cli
   :no-imported-members:
