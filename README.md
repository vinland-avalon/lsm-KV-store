<!--
 * @Author: BohanWu 819186192@qq.com
 * @Date: 2022-11-30 10:24:05
 * @LastEditors: BohanWu 819186192@qq.com
 * @LastEditTime: 2022-12-07 18:49:19
 * @FilePath: /lsm-KV-store/README.md
 * @Description: 
 * 
 * Copyright (c) 2022 by BohanWu 819186192@qq.com, All Rights Reserved. 
-->
<h1 align="center">
  <br>
  LSM-KV-STORE
  <br>
</h1>

<p align="center">
  <a href="#key-features">Key Features</a> •
  <a href="#how-to-use">How To Use</a> •
  <a href="#download">Download</a> •
  <a href="#credits">Credits</a> •
  <a href="#related">Related</a> •
  <a href="#license">License</a>
</p>

## Key Features

* Memtables, including an active memtable and an immutable memtable
  - implemented both in skip-list and red-black-tree
* Sorted-string-table
  -  bloom filter + prefix compression (in development) + restart point  
* Linux/C++ 
* supported by CMake
* one-layer sstable so far
* memTable + immutableMemTable
* header-only

## How To Use

To clone and run this application, you'll need [Git](https://git-scm.com) From your command line:

```bash
# to try such codes,
# you can now run the unittests in test.cpp (not test folder).
# There's a way to run the tests
# 1. make a new directory named ./build
# 2. under ./build, cmake ..
# 3. under ./build, create ./data
# 4. cmake --build .
# 5. ./FooTest
```

> **Note**
>: not ready for install, but nearly.


## Download

You can [download](https://github.com/vinland-avalon/lsm-KV-store) the latest version.

## Emailware

I'd like you send me an email at <bohanwu1022@gmail.com> or <819186192@qq.com> about anything you'd want to say about this program. I'd really appreciate it!

## Credits

This software uses the following open source packages:

- [nlohmann-json](https://github.com/nlohmann/json): easy to use -- header-only
- [googletest](https://github.com/google/googletest.git): alongwith googlemock 
- [spdlog](https://github.com/gabime/spdlog.git): for error, warn, info messages, clear and concurrent safe

## Related

...

## License

MIT

---

> GitHub [@vinland-avalon](https://github.com/vinland-avalon) &nbsp;&middot;&nbsp;
> LinkedIn [@Bohan Wu](https://www.linkedin.com/in/bohan-wu-044887244/)
