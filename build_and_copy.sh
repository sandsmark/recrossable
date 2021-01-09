#!/bin/bash
docker run -ti --rm -v `pwd`:/app ghcr.io/toltec-dev/qt:v1.2.2 /app/build.sh
scp recrossable root@remarkable:/opt/bin/recrossable
