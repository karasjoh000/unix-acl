#!/usr/bin/env bash
make clean
make
sudo chown acluser get
sudo chgrp acluser get
sudo chmod u+s get
