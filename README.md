# CodeSnippets
This project, contains a list of code snippets for several purpose

## Nested Vectors Recursion
The [NestedVectorsRecursion](https://github.com/shogunxam/CodeSnippets/blob/90857a8ddbaa7d11d7c301312c14f75ca3a7ecbb/NestedVectorsRecursion.cpp) module contains an implementation of two template functions to recursivelly modify or print the content of nested std::vectors.
The two methods are impelemented using the SFINAE ( Substitution Failure Is Not An Error ) principle.

## Customize the name of the memeber of a std::pair 
The [Custompair](https://github.com/shogunxam/CodeSnippets/blob/e68aaecaaf4f13911a880fa54fb85c8e928663bd/CustomPair.cpp) module contains an implementation of a macro to create a new std::pair struct with cutom members' names. The members std::pair::first and std::pair::second are made private and they are exposed using references.
