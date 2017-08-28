#!/bin/sh

#
# MetaCall Library by Parra Studios
# Copyright (C) 2016 - 2017 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
#

# Configure compose variables
export COMPOSE_PROJECT_NAME='metacall'

# Configure variables
export METACALL_PATH=/usr/local/metacall

# Build docker compose
if [ "$1" = 'build' ]; then
	docker-compose build
fi

# Run docker compose
if [ "$1" = 'up' ]; then
	docker-compose up
fi

# Stop docker compose
if [ "$1" = 'stop' ]; then
	docker-compose stop
fi

# Down docker compose
if [ "$1" = 'down' ]; then
	docker-compose down
fi
