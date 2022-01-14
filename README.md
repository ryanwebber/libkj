# LibKJ

An excellent C++ utility library used in Cap'n Proto that has been extracted into its
own github repository here.

## Building

The KJ libraries can be built out-of-source with CMake:
```bash
git clone git@github.com:ryanwebber/libkj.git
cd libkj
mkdir build
cd build
cmake ../
make -j 4
```

This will produce the following static libraries:
 * libkj-async
 * libkj-gzip
 * libkj-http
 * libkj-test
 * libkj-tls
 * libkj

## Contributing

Please contribute work to the source in the core Cap'n Proto repository according to
their contribution guidelines. Sane maintenance changes (updating to new Cap'n Proto
versions, fixing build issues, etc) are gratefully welcome here!

## Licence
MIT - just as the Cap'n Proto source. A copy of the original licence and all copywrite
notices have been retained accordingly.

