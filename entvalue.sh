#! /bin/bash
ent $1 | grep  'Entropy' | awk '{print $3}'
