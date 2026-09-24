# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `get_Benders_form()` and `give_back_Benders_form()`: the same problem in the
  form a Benders decomposition asks for, i.e., a root holding a single copy of
  the static here-and-now Variable, integer where they are, with their box and
  the cost of the design, and one sub-Block per leaf holding the leaf and the
  coupling x^l - x <= 0; the leaves are moved into it, not copied, and are
  given back, with the cost of the design written back into them, when the
  form is deleted

### Changed

- whoever links the module keeps it: the classes of a module register
  themselves in the factory from a static initialiser, and a linker that
  drops what looks unused takes the registration away with it, so the target
  now tells whoever links it to keep the symbol that forces the module in,
  and on ELF, where naming the symbol is not enough, the library as a whole

### Fixed

- the header includes the `Solution` it derives from, which it was getting by
  chance from another header and would have stopped getting the day that one
  stopped including it
## [0.2.0] - 2026-09-12

### Changed

- the first-stage AbstractPath are optional: without them the Block has no
  here-and-now Variable of its own

- the leaves of the scenario structure have a count and an accessor of their
  own, since a nested structure has more of them than it has scenarios

- the version of the module is the git tag of its repository, or the
  VERSION.txt of a release tarball, and the shared library carries it: its
  SONAME is major.minor while the major is 0, and it is installed with an
  RPATH relative to itself, so that an installed tree keeps working wherever
  it is moved

### Fixed

- the scenario copies keep this Block as their father: the StochasticBlock
  serving as the applicator was left in the chain, and silently dropped every
  Modification coming from a scenario

- `serialize()` writes a TwoStageStochasticBlock that `deserialize()` reads
  back

## [0.1.0] - 2025-12-12

### Added

- First test release.

[Unreleased]: https://gitlab.com/smspp/twostagestochasticblock/-/compare/0.2.0...develop
[0.2.0]: https://gitlab.com/smspp/twostagestochasticblock/-/compare/0.1.0...0.2.0
[0.1.0]: https://gitlab.com/smspp/twostagestochasticblock/-/tags/0.1.0
