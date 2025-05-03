# CodeSnippets
This project, contains a list of code snippets for several purpose

## Interruptible Thread
The [InterruptibleThread](InterruptibleThread.cpp) modules contains an implementation of Thread class. The thread is made interruptble using interruption checkpoint in the running function.

## Nested Vectors Recursion
The [NestedVectorsRecursion](NestedVectorsRecursion.cpp) module contains an implementation of two template functions to recursivelly modify or print the content of nested std::vectors.
The two methods are impelemented using the SFINAE ( Substitution Failure Is Not An Error ) principle.

## Customize the name of the member of a std::pair 
The [Custompair](CustomPair.cpp) module contains an implementation of a macro to create a new std::pair struct with cutom members' names. The members std::pair::first and std::pair::second are made private and they are exposed using references.

## A container for any type
The [Any](Any.cpp) module contains an implementation of a container for any type. It's a good replacement of the std::any class introduced with C++17 in case you are obliged to use an older C++ version.

## A memory binary input stream
The [MemoryBinaryStream](MemoryBinaryStream.cpp) module contains an implementation of an in memory binary stream. It can be used to store data directly in a std::vector where an input stream is required.

## Support for UTF-8 path in Microsoft Windows
The [UTF8](UTF8.cpp) module contains a wrapper for the fstream classes to open files with a path UTF-8 encoded.
