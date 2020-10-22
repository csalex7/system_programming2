#!/bin/bash
awk 'BEGIN{
FS=":" #delimiter
n=999
}
{
if($3 != " ")
{
  sum[$3]++
  if (sum[$3]<=n && sum[$3]>0) #ta keywords vriskontai meta to 2o :
  {
    n=sum[$3]
    m=$3
  }
  }
}
END{
print m,"total times found:"n
}' log/*
