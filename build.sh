#!/bin/bash

DOCKERCMD=${DOCKER:-sudo docker}
BASENAME=u18-fuzzing-crypto
CANARY=.container_run
$DOCKERCMD container stop  ${BASENAME}-container
$DOCKERCMD container rm    ${BASENAME}-container
$DOCKERCMD rmi             ${BASENAME}-image

if [ -f "${BASENAME}-image.tar" ]; then
	$DOCKERCMD load < ${BASENAME}-image.tar
else
	$DOCKERCMD build -t        ${BASENAME}-image .
fi
rm -f $CANARY
