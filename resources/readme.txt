testme:

This program is a test program that prints out the instance, the total number of instances, and the range of values each instance processes.

Usage: testme <totalNumInstance> <index> <values>

<totalNumInstance>: this is the total number of instances the shell launches; the number must be positive
<index>: this is the index of the instance; the number must be zero to <total_numInstances> - 1
<value>: the values processed by all instnaces; the number must be a positive value

prime:

This program generates all prime numbers that occur within a range of numbers. The range is determined by the index of the instance
and the specified largest number. The program uses divide-and-conquer to compute prime numbers across all instances within the total specified range.

Usage: prime <totalNumInstances> <index> <upperValue>

<totalNumInstance>: this is the total number of instances the shell launches; the number must be positive
<index>: this is the index of the instance; the number must be zero to <total_numInstances> - 1
<upperValue>: the largest value to be tested for primality; all numbers between 1 and <upperValue> will be tested for primality
