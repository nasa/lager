# How to contribute

Thanks for contributing!

Lager is maintained on an internal Gitlab server at NASA's Johnson Space Center.  As a result, this Github repository is a one way mirror of that internal repository.  To contribute:

1. Fork this repository to your personal account.
2. Create a branch and make your changes.
3. Test the changes in your fork.
4. Submit a pull request to open discussion about your changes.
5. Maintainers will review the pull request and merge it internally.
6. Changes will be mirrored back out to the Github repository.

## Goals

The general goal of Lager is to be a light weight (i.e. few dependencies), high performance logging system.  See the [design document](doc/design.md) for details.

## Testing

Lager has extensive unit tests using the Googletest framework.  Please continue use of this framework and ensure the unit tests pass by running `make test`.  There's also a `cppcheck` target so please also ensure `make cppcheck` passes.

## Coding conventions

We generally follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html), so please conform to this style and don't make major file formatting changes.

## License

The LICENSE file in this repository contains the licensing information for Lager.  By submitting a pull request, you are agreeing to allow distribution of your work under this license.
