# pyzz

pyzz is a python interface for [ABC/ZZ](https://bitbucket.org/niklaseen/abc-zz)

## Build

1. Checkout ABC/ZZ

```
hg clone https://bitbucket.org/niklaseen/abc-zz
```

2. Change directory into `abc-zz` and checkout [pyzz](https://bitbucket.org/sterin/pyzz)

```
cd abc-zz
hg clone https://bitbucket.org/sterin/pyzz
```

3. Change direcotry into `pyzz/api` and checkout [pywrapper](https://bitbucket.org/sterin/pywrapper)

```
cd pyzz/api
hg clone https://bitbucket.org/sterin/pywrapper
```
  
4. Build `pyzz`. It should generate and executable named `out/<machine>/release_i/pyzz.exe`, it is a python interpreter with the `pyzz` module embedded in it

```
../../BUILD/zb ri
```

5. Now build and install the python extension

```
python setup.py build
python setup.py install --user
```
