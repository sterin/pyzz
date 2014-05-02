# pyzz

pyzz is a python interface for Niklas Een's ABC/ZZ

## Build

1. Checkout ABC/ZZ
  ```
  hg clone https://bitbucket.org/niklaseen/abc-zz
  ```
2. Change directory into ABC/ZZ and checkout pyzz
  ```
  cd abc-zz
  hg clone https://bitbucket.org/sterin/pyzz
  ```
3. Change direcotry into pyzz/api and checkout pywrapper
  ```
  cd pyzz/api
  hg clone https://bitbucket.org/sterin/pywrapper
  ```
  
4. Build pyzz, a Python interpretere executable with pyzz embedded in it
  ```
  ../../BUILD/zb
  ```
  The result is an executable pyzz.exe, in out/<machine>/<configuration>
