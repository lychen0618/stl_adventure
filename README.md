# stl_adventure
参照《STL源码剖析》，实现了STL的部分内容。

目前实现的内容有：
* 两层的空间配置器（memory/alloc.h），第一层的`malloc_allocator`是对`operator new和operator delete`的简单封装，第二层的`pool_allocator`是一个简单的基于内存池的空间配置器。
* STL容器`vector`（container/sad_vector.h），它的内存分配基于上面的两层式空间配置器。
* `unordered_map`等容器的底层数据结构——哈希表（container/sad_hashtable.h）

TODO：
* fix bug。目前从`container\tests\test_hashtable.cpp`得到的可执行文件会出现coredump，需要使用gdb去调试；使用内存分析工具检测是否会出现内存泄漏。
* 增加对`sad_vector`和`sad_hashtable`的测试，目前只是简单测试了一下。
* 实现`map`，`set`的底层数据结构——红黑树。

