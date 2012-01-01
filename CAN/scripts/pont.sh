mkfifo bin
nc 127.0.0.1 $1 < bin | converter -i bin -o dec | nc 127.0.0.1 $2 | converter -i dec -o bin > bin
