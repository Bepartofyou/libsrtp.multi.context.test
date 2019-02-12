#!/bin/sh
make

./bin/srtp-2.0.0  ./data/2000000.data.srtp  QLGPC4bP/Z1WZx4aYp26afwW6qLUvhk3Su/CCKnOMBWAj0FsX1o1lfffSmA= > ./logs/srtp-2.0.0.log
./bin/srtp-2.1.0  ./data/2010000.data.srtp  QLGPC4bP/Z1WZx4aYp26afwW6qLUvhk3Su/CCKnOMBWAj0FsX1o1lfffSmA= > ./logs/srtp-2.1.0.log
./bin/srtp-2.2.0  ./data/2020000.data.srtp  QLGPC4bP/Z1WZx4aYp26afwW6qLUvhk3Su/CCKnOMBWAj0FsX1o1lfffSmA= > ./logs/srtp-2.2.0.log
./bin/srtp-master ./data/2030000.data.srtp  QLGPC4bP/Z1WZx4aYp26afwW6qLUvhk3Su/CCKnOMBWAj0FsX1o1lfffSmA= > ./logs/srtp-master.log