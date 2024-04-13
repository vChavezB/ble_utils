# Zephyr BLE Utils Module

![version](https://img.shields.io/badge/version-1.0.0-blue)

This [Zephyr module](https://docs.zephyrproject.org/3.2.0/develop/modules.html) includes utilities to develop BLE GATT Services and Characteristics with a C++ syntax.

## Requirements

- C++17
- Zephyr OS >=v3.2.x

## Motivation

The  definition of BLE services and characteristics can be done at compile time with the Zephyr C Macros located in `<zephyr/bluetooth/gatt.h>`. These macros rely heavily on C syntax and use of temporary structures which is not 100 % compatible with C++. In addition, as far as I know there is no documentation on how to use directly the struct `bt_gatt_attr` and the only source of information is reading and decoding how the zephyr C macros initialize the characteristics and services.

## Features

- C++ compatible
- Callbacks through inheritance
- Easy integration into C++ applications.
- Abstraction of the current undocumented struct `bt_gatt_attr`.


## How to use

A demo is located in the samples folder. This demo can be used as a reference on how to use this module.


# Tests

Some basic tests are done with [Renode](https://renode.readthedocs.io/en/latest/) to simulate the correct setup of the BLE Services and characteristics. 

## Requirements

- Renode v1.13.3
- Robot framework

## Robot

To run the unit test suite with robot execute:

```bash
renode-test -t tests/renode/uptime_test.robot
```

## Debug 

If you want to manually debug the tests instead load the Renode script

```bash
renode tests/renode/uptime.resc
```


## Contact

Contact for issues, contributions as git patches or general information at vchavezb(at)protonmail.com

