docker-image:
	docker-compose build libswiftnav

docker: docker-image
	docker-compose run libswiftnav

docker-build: docker-image
	mkdir -p build
	docker-compose run -T libswiftnav /bin/bash -c "cd build && cmake .. && make -j4"

docker-lint: docker-image
	mkdir -p build
	docker-compose run -T libswiftnav /bin/bash -c "cd build && cmake .. && make -j4 clang-format-all"
