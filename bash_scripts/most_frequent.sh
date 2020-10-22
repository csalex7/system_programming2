#!/bin/bash
awk 'BEGIN{
FS=":" #delimiter
n=0
}
{
sum[$3]++
if (sum[$3]>n) #ta keywords vriskontai meta to 2o :
{
  n=sum[$3]
  m=$3
}
}
END{
print m,"total times found:"n
#print n
}' log/*
