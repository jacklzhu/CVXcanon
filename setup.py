"""CVXcanon module.

Convenient development to recompile everything and run tests:
  cd ~/CVXcanon/src &&
  swig -python -c++ python/CVXcanon.i &&
  mv python/CVXcanon_wrap.cxx python/CVXcanon_wrap.cpp &&
  cd ~/CVXcanon &&
  CC="ccache clang" CXX="ccache clang++" python setup.py build &&
  python setup.py -q develop --user &&
  nosetests cvxcanon

TODO(mwytock): Encapsulate swig/C++ compilation steps in Makefile?
"""

from setuptools import setup, Extension, find_packages
from setuptools.command.install import install
from distutils.command.build import build
import numpy
import os

SCS_DIR = "third_party/scs/"

canon = Extension(
    name="_CVXcanon",
    language="c++",
    extra_compile_args=["-std=c++14"],
    sources=[
        "src/CVXcanon.cpp",
        "src/Expression.cpp",
        "src/ExpressionShape.cpp",
        "src/ExpressionUtil.cpp",
        "src/LinOpOperations.cpp",
        "src/LinearConeTransform.cpp",
        "src/LinearExpression.cpp",
        "src/MatrixUtil.cpp",
        "src/Solver.cpp",
        "src/SplittingConeSolver.cpp",
        "src/SymbolicConeSolver.cpp",
        "src/TextFormat.cpp",
        "src/Utils.cpp",
        "src/python/CVXcanon_wrap.cpp",
    ],
    include_dirs=[
        "src",
        "src/python",
        "include",
        numpy.get_include(),
        "third_party",
    ],
    extra_link_args=[
        "third_party/scs/out/libscsdir.a",
    ],
)

base_dir = os.path.dirname(__file__)
about = {}
with open(os.path.join(base_dir, "src", "python", "_version__.py")) as f:
    exec(f.read(), about)

setup(
    name="CVXcanon",
    version=about["__version__"],
    setup_requires=["numpy"],
    author="Jack Zhu, John Miller, Paul Quigley",
    author_email="jackzhu@stanford.edu, millerjp@stanford.edu, piq93@stanford.edu",
    ext_modules=[canon],
    package_dir={"": "src/python"},
    py_modules=["canonInterface", "CVXcanon", "_version__"],
    description="A low-level library to perform the matrix building step in cvxpy, a convex optimization modeling software.",
    license="GPLv3",
    url="https://github.com/jacklzhu/CVXcanon",
    install_requires=[
        "numpy",
        "scipy",
    ]
)
