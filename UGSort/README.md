THE UGSort APPLICATION

The UGSort application is a testbed for an implementation of the UGSort merge sort algorithm.
The application will sort text files based on a fixed length ascii key at a given offset in each record in the unsorted file.
Sorted output will be written to a designated output file.
The implementation is minimally optimised providing indicative timing for any implementation of the algorithm.
The application is minimally instrumented to provide the ability to perform timing comparisons for different scenarios.
The application is intended for use on Windows and Linux deployments.

This implementation supports in-memory or on-disk sorting, output in ascending or descending sequence and optionally key stability.

v1.15 Uses an optimised UGSort algorithm that uses a binary chop for searching the array of splitter stores.
