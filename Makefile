all: build

build:
	g++ -shared -fPIC -o bg3_linux_ae.so bg3_linux_ae.cpp -ldl -std=c++11

clean:
	-rm -fr release/*

release: docker-build docker-run docker-get

docker-build:
	docker build -t bg3_linux_ae_build . -f Dockerfile

docker-get:
	docker cp bg3_linux_ae_run:/app/bg3-linux-ae.tar.gz ./release/

docker-run:
	$(info docker cp bg3_linux_ae:/app/bg3-linux-ae.tar.gz ./)
	-docker rm bg3_linux_ae_run
	docker run --name bg3_linux_ae_run \
	-it bg3_linux_ae_build
