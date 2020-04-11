docker build -t todaylg/envtools ./

docker run -t -i todaylg/envtools /bin/bash

docker ps -a

docker container ls -a

docker container start/stop containerID

docker exec -it containerID bash

docker container prune

docker run -v C:\Gallery\Code\Github\envTools/data:/data -t todaylg/envtools envBRDF -s 128 -n 1024 /data/result/brdf_ue4.bin