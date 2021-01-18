#!/bin/sh
if stat $(echo $0 | tr '[a-zA-Z]' '[A-Za-z]') &> /dev/null; then
	exit 1
else
	exit 0
fi
