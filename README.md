# vectorDB-client supported by MongoDB
This is a learning project for using MongoDB c++ driver and BSON document support.
## Prerequisites
Connection URI should be obtained by deploy MongoDB database by Atlas or locally MongoDB（mongosh).

But MongoDB Atlas Vector Search supports native vector search(HNSW).

MongoDB（mongosh) needs third library(FAISS, HNSWlib) to support ANN(Approximate Nearest Neighbor) vector search.
##Build
``` sh
$ cd thirdparty
$ bash build.sh
```
switch to root directory.
``` sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```
using google test.
``` sh
$ cd test
$ mkdir build
$ cd build
$ cmake ..
$ make
$./vector_store_test
```
